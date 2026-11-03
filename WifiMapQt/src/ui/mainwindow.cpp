#include "mainwindow.h"

#include "mapcanvas.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QKeySequence>
#include <QJsonParseError>
#include <QLabel>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSaveFile>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QUndoCommand>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <functional>
#include <stdexcept>

namespace {
class SnapshotCommand final : public QUndoCommand {
public:
    SnapshotCommand(MapModel &model, QJsonObject before, QJsonObject after,
                    std::function<void()> refresh, const QString &label)
        : m_model(model), m_before(std::move(before)), m_after(std::move(after)),
          m_refresh(std::move(refresh))
    {
        setText(label);
    }

    void undo() override
    {
        m_model.loadJson(m_before);
        m_refresh();
    }

    void redo() override
    {
        m_model.loadJson(m_after);
        m_refresh();
    }

private:
    MapModel &m_model;
    QJsonObject m_before;
    QJsonObject m_after;
    std::function<void()> m_refresh;
};

QSpinBox *integerSpinBox(int maximum, QWidget *parent)
{
    auto *spin = new QSpinBox(parent);
    spin->setRange(0, maximum);
    return spin;
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_model(500, 500), m_undoStack(new QUndoStack(this))
{
    setWindowTitle(tr("WifiMap"));
    resize(1280, 820);
    createUi();
    createMenus();
    statusBar()->showMessage(tr("Ready"));
    QTimer::singleShot(0, m_canvas, [this] { m_canvas->fitToView(); });
}

void MainWindow::createUi()
{
    auto *splitter = new QSplitter(this);
    auto *controls = new QWidget(splitter);
    auto *layout = new QVBoxLayout(controls);

    auto *pointGroup = new QGroupBox(tr("Wi-Fi access point"), controls);
    auto *pointForm = new QFormLayout(pointGroup);
    m_pointX = integerSpinBox(MapModel::maximumDimension - 1, pointGroup);
    m_pointY = integerSpinBox(MapModel::maximumDimension - 1, pointGroup);
    m_frequency = new QComboBox(pointGroup);
    m_frequency->addItem(QStringLiteral("2.4 GHz"), 2.4);
    m_frequency->addItem(QStringLiteral("5 GHz"), 5.0);
    m_power = new QSpinBox(pointGroup);
    m_power->setRange(11, 20);
    m_pointX->setObjectName(QStringLiteral("pointX"));
    m_pointY->setObjectName(QStringLiteral("pointY"));
    m_power->setValue(11);
    pointForm->addRow(tr("X"), m_pointX);
    pointForm->addRow(tr("Y"), m_pointY);
    pointForm->addRow(tr("Frequency"), m_frequency);
    pointForm->addRow(tr("Power (dBm)"), m_power);
    auto *pointButtons = new QHBoxLayout;
    m_addPoint = new QPushButton(tr("Add"), pointGroup);
    m_updatePoint = new QPushButton(tr("Update"), pointGroup);
    m_deletePoint = new QPushButton(tr("Delete"), pointGroup);
    pointButtons->addWidget(m_addPoint);
    pointButtons->addWidget(m_updatePoint);
    pointButtons->addWidget(m_deletePoint);
    pointForm->addRow(pointButtons);
    m_updatePoint->setEnabled(false);
    m_deletePoint->setEnabled(false);
    layout->addWidget(pointGroup);

    auto *barrierGroup = new QGroupBox(tr("Barrier"), controls);
    auto *barrierForm = new QFormLayout(barrierGroup);
    m_x1 = integerSpinBox(MapModel::maximumDimension - 1, barrierGroup);
    m_y1 = integerSpinBox(MapModel::maximumDimension - 1, barrierGroup);
    m_x2 = integerSpinBox(MapModel::maximumDimension - 1, barrierGroup);
    m_y2 = integerSpinBox(MapModel::maximumDimension - 1, barrierGroup);
    m_thickness = new QSpinBox(barrierGroup);
    m_thickness->setRange(1, 499);
    m_thickness->setValue(30);
    m_material = new QComboBox(barrierGroup);
    for (WallMaterial material : {WallMaterial::Plaster, WallMaterial::Wood, WallMaterial::Brick,
                                  WallMaterial::Cement, WallMaterial::ReinforcedConcrete, WallMaterial::Metal})
        m_material->addItem(MapModel::materialName(material), static_cast<int>(material));
    barrierForm->addRow(tr("Start X"), m_x1);
    barrierForm->addRow(tr("Start Y"), m_y1);
    barrierForm->addRow(tr("End X"), m_x2);
    barrierForm->addRow(tr("End Y"), m_y2);
    barrierForm->addRow(tr("Thickness (cm)"), m_thickness);
    barrierForm->addRow(tr("Material"), m_material);
    auto *barrierButtons = new QHBoxLayout;
    m_addBarrier = new QPushButton(tr("Add"), barrierGroup);
    m_updateBarrier = new QPushButton(tr("Update"), barrierGroup);
    m_deleteBarrier = new QPushButton(tr("Delete"), barrierGroup);
    barrierButtons->addWidget(m_addBarrier);
    barrierButtons->addWidget(m_updateBarrier);
    barrierButtons->addWidget(m_deleteBarrier);
    barrierForm->addRow(barrierButtons);
    auto *drawBarrier = new QPushButton(tr("Draw barrier on map"), barrierGroup);
    drawBarrier->setCheckable(true);
    barrierForm->addRow(drawBarrier);
    m_updateBarrier->setEnabled(false);
    m_deleteBarrier->setEnabled(false);
    layout->addWidget(barrierGroup);

    auto *mapGroup = new QGroupBox(tr("Map and display"), controls);
    auto *mapForm = new QFormLayout(mapGroup);
    m_mapLength = new QSpinBox(mapGroup);
    m_mapWidth = new QSpinBox(mapGroup);
    m_mapLength->setRange(MapModel::minimumDimension, MapModel::maximumDimension);
    m_mapWidth->setRange(MapModel::minimumDimension, MapModel::maximumDimension);
    m_mapLength->setValue(m_model.length());
    m_mapWidth->setValue(m_model.width());
    mapForm->addRow(tr("Length"), m_mapLength);
    mapForm->addRow(tr("Width"), m_mapWidth);
    auto *resizeButton = new QPushButton(tr("Resize map"), mapGroup);
    auto *newMapButton = new QPushButton(tr("New map"), mapGroup);
    mapForm->addRow(resizeButton, newMapButton);
    auto *grid = new QCheckBox(tr("Show grid"), mapGroup);
    auto *magnetism = new QCheckBox(tr("Snap to grid"), mapGroup);
    auto *gridSize = new QSpinBox(mapGroup);
    gridSize->setRange(1, 50);
    gridSize->setValue(10);
    mapForm->addRow(grid);
    mapForm->addRow(magnetism);
    mapForm->addRow(tr("Grid spacing (m)"), gridSize);
    auto *fitButton = new QPushButton(tr("Fit map to view"), mapGroup);
    mapForm->addRow(fitButton);
    layout->addWidget(mapGroup);

    m_selectionLabel = new QLabel(tr("Nothing selected"), controls);
    m_selectionLabel->setWordWrap(true);
    layout->addWidget(m_selectionLabel);
    layout->addStretch();
    controls->setMinimumWidth(300);
    controls->setMaximumWidth(360);

    m_canvas = new MapCanvas(m_model, splitter);
    splitter->addWidget(controls);
    splitter->addWidget(m_canvas);
    splitter->setStretchFactor(1, 1);
    setCentralWidget(splitter);

    connect(m_addPoint, &QPushButton::clicked, this, [this] {
        commitChange(tr("Add access point"), [this] {
            m_selectedId = m_model.addAccessPoint(m_pointX->value(), m_pointY->value(),
                                                   m_frequency->currentData().toDouble(), m_power->value());
            m_selectedBarrier = false;
        });
    });
    connect(m_updatePoint, &QPushButton::clicked, this, [this] {
        if (m_selectedId < 0 || m_selectedBarrier)
            return;
        commitChange(tr("Update access point"), [this] {
            m_model.updateAccessPoint(m_selectedId, m_pointX->value(), m_pointY->value(),
                                      m_frequency->currentData().toDouble(), m_power->value());
        });
    });
    connect(m_deletePoint, &QPushButton::clicked, this, [this] {
        if (m_selectedId < 0 || m_selectedBarrier)
            return;
        const int id = m_selectedId;
        commitChange(tr("Delete access point"), [this, id] { m_model.removeAccessPoint(id); });
        selectElement(-1);
    });
    connect(m_addBarrier, &QPushButton::clicked, this, [this] {
        commitChange(tr("Add barrier"), [this] {
            m_selectedId = m_model.addBarrier(m_x1->value(), m_y1->value(), m_x2->value(), m_y2->value(),
                                               m_thickness->value(),
                                               static_cast<WallMaterial>(m_material->currentData().toInt()));
            m_selectedBarrier = true;
        });
    });
    connect(m_updateBarrier, &QPushButton::clicked, this, [this] {
        if (m_selectedId < 0 || !m_selectedBarrier)
            return;
        commitChange(tr("Update barrier"), [this] {
            m_model.updateBarrier(m_selectedId, m_x1->value(), m_y1->value(), m_x2->value(), m_y2->value(),
                                  m_thickness->value(),
                                  static_cast<WallMaterial>(m_material->currentData().toInt()));
        });
    });
    connect(m_deleteBarrier, &QPushButton::clicked, this, [this] {
        if (m_selectedId < 0 || !m_selectedBarrier)
            return;
        const int id = m_selectedId;
        commitChange(tr("Delete barrier"), [this, id] { m_model.removeBarrier(id); });
        selectElement(-1);
    });
    connect(drawBarrier, &QPushButton::toggled, this, [this](bool checked) {
        m_canvas->setBarrierDrawing(checked);
        statusBar()->showMessage(checked ? tr("Drag on the map to draw a horizontal or vertical barrier")
                                         : tr("Ready"));
    });
    connect(resizeButton, &QPushButton::clicked, this, [this] {
        commitChange(tr("Resize map"), [this] {
            m_model.resize(m_mapLength->value(), m_mapWidth->value());
        });
        m_canvas->fitToView();
    });
    connect(newMapButton, &QPushButton::clicked, this, &MainWindow::createNewMap);
    connect(grid, &QCheckBox::toggled, m_canvas, &MapCanvas::setGridVisible);
    connect(magnetism, &QCheckBox::toggled, m_canvas, [this](bool enabled) {
        m_canvas->setMagnetismEnabled(enabled);
    });
    connect(gridSize, &QSpinBox::valueChanged, m_canvas, &MapCanvas::setGridSize);
    connect(fitButton, &QPushButton::clicked, m_canvas, &MapCanvas::fitToView);

    m_canvas->cellClicked = [this](int x, int y) {
        m_pointX->setValue(x);
        m_pointY->setValue(y);
        m_x1->setValue(x);
        m_y1->setValue(y);
        m_x2->setValue(x);
        m_y2->setValue(y);
    };
    m_canvas->selectionChanged = [this](int id) { selectElement(id); };
    m_canvas->barrierDrawn = [this](int x1, int y1, int x2, int y2) {
        m_x1->setValue(x1);
        m_y1->setValue(y1);
        m_x2->setValue(x2);
        m_y2->setValue(y2);
        commitChange(tr("Draw barrier"), [this, x1, y1, x2, y2] {
            m_selectedId = m_model.addBarrier(x1, y1, x2, y2, m_thickness->value(),
                                               static_cast<WallMaterial>(m_material->currentData().toInt()));
            m_selectedBarrier = true;
        });
    };
    m_canvas->accessPointMoved = [this](int id, int x, int y) {
        try {
            m_model.moveAccessPoint(id, x, y);
            refreshCanvas();
        } catch (const std::exception &error) {
            statusBar()->showMessage(QString::fromUtf8(error.what()), 3000);
        }
    };
    m_canvas->barrierMoved = [this](int id, int x1, int y1, int x2, int y2) {
        try {
            m_model.moveBarrier(id, x1, y1, x2, y2);
            refreshCanvas();
        } catch (const std::exception &error) {
            statusBar()->showMessage(QString::fromUtf8(error.what()), 3000);
        }
    };
    m_canvas->dragStarted = [this](int, bool barrier) {
        m_dragBefore = m_model.toJson();
        m_dragLabel = barrier ? tr("Move barrier") : tr("Move access point");
        m_dragActive = true;
    };
    m_canvas->dragFinished = [this] { finishDrag(); };
}

void MainWindow::createMenus()
{
    QMenu *file = menuBar()->addMenu(tr("&File"));
    QAction *newAction = file->addAction(tr("&New map"));
    QAction *openAction = file->addAction(tr("&Open…"));
    QAction *saveAction = file->addAction(tr("&Save"));
    QAction *saveAsAction = file->addAction(tr("Save &as…"));
    QAction *exportAction = file->addAction(tr("Export &PNG…"));
    file->addSeparator();
    QAction *quitAction = file->addAction(tr("&Quit"));

    QMenu *edit = menuBar()->addMenu(tr("&Edit"));
    edit->addAction(m_undoStack->createUndoAction(this, tr("&Undo")));
    edit->addAction(m_undoStack->createRedoAction(this, tr("&Redo")));
    edit->addSeparator();
    m_copyAction = edit->addAction(tr("&Copy element"));
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setShortcutContext(Qt::WidgetShortcut);
    m_copyAction->setEnabled(false);
    m_pasteAction = edit->addAction(tr("&Paste element"));
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setShortcutContext(Qt::WidgetShortcut);
    m_pasteAction->setEnabled(false);
    m_canvas->addAction(m_copyAction);
    m_canvas->addAction(m_pasteAction);

    connect(newAction, &QAction::triggered, this, &MainWindow::createNewMap);
    connect(openAction, &QAction::triggered, this, &MainWindow::openMap);
    connect(saveAction, &QAction::triggered, this, [this] { saveMap(); });
    connect(saveAsAction, &QAction::triggered, this, [this] { saveMapAs(); });
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportPng);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copySelectedElement);
    connect(m_pasteAction, &QAction::triggered, this, &MainWindow::pasteElement);
}

void MainWindow::selectElement(int id)
{
    m_selectedId = id;
    m_selectedBarrier = false;
    if (id >= 0) {
        const auto point = std::find_if(m_model.accessPoints().cbegin(), m_model.accessPoints().cend(),
                                        [id](const AccessPoint &item) { return item.id == id; });
        if (point != m_model.accessPoints().cend()) {
            m_selectedBarrier = false;
            m_pointX->setValue(point->x);
            m_pointY->setValue(point->y);
            m_frequency->setCurrentIndex(point->frequencyGHz == 2.4 ? 0 : 1);
            m_power->setValue(point->powerDbm);
            m_selectionLabel->setText(tr("Selected access point #%1").arg(id));
        } else {
            const auto barrier = std::find_if(m_model.barriers().cbegin(), m_model.barriers().cend(),
                                              [id](const Barrier &item) { return item.id == id; });
            if (barrier != m_model.barriers().cend()) {
                m_selectedBarrier = true;
                m_x1->setValue(barrier->x1);
                m_y1->setValue(barrier->y1);
                m_x2->setValue(barrier->x2);
                m_y2->setValue(barrier->y2);
                m_thickness->setValue(barrier->thicknessCm);
                m_material->setCurrentIndex(m_material->findData(static_cast<int>(barrier->material)));
                m_selectionLabel->setText(tr("Selected barrier #%1").arg(id));
            } else {
                m_selectedId = -1;
            }
        }
    }
    m_updatePoint->setEnabled(m_selectedId >= 0 && !m_selectedBarrier);
    m_deletePoint->setEnabled(m_selectedId >= 0 && !m_selectedBarrier);
    m_updateBarrier->setEnabled(m_selectedId >= 0 && m_selectedBarrier);
    m_deleteBarrier->setEnabled(m_selectedId >= 0 && m_selectedBarrier);
    if (m_copyAction)
        m_copyAction->setEnabled(m_selectedId >= 0);
    if (m_selectedId < 0)
        m_selectionLabel->setText(tr("Nothing selected"));
    m_canvas->setSelection(m_selectedId, m_selectedBarrier);
}

void MainWindow::refreshCanvas()
{
    m_canvas->refresh();
    if (m_selectedId >= 0)
        selectElement(m_selectedId);
}

void MainWindow::reportError(const QString &message)
{
    statusBar()->showMessage(message, 7000);
    QMessageBox::warning(this, tr("WifiMap"), message);
}

void MainWindow::commitChange(const QString &label, const std::function<void()> &change)
{
    const QJsonObject before = m_model.toJson();
    try {
        change();
        const QJsonObject after = m_model.toJson();
        m_model.loadJson(before);
        m_undoStack->push(new SnapshotCommand(m_model, before, after,
                                              [this] { refreshCanvas(); }, label));
        refreshCanvas();
        statusBar()->showMessage(label, 3000);
    } catch (const std::exception &error) {
        m_model.loadJson(before);
        refreshCanvas();
        reportError(QString::fromUtf8(error.what()));
    }
}

void MainWindow::copySelectedElement()
{
    if (m_selectedId < 0)
        return;

    if (m_selectedBarrier) {
        const auto it = std::find_if(m_model.barriers().cbegin(), m_model.barriers().cend(),
                                     [this](const Barrier &barrier) { return barrier.id == m_selectedId; });
        if (it == m_model.barriers().cend())
            return;
        m_copiedBarrier = *it;
        m_copiedBarrierElement = true;
    } else {
        const auto it = std::find_if(m_model.accessPoints().cbegin(), m_model.accessPoints().cend(),
                                     [this](const AccessPoint &point) { return point.id == m_selectedId; });
        if (it == m_model.accessPoints().cend())
            return;
        m_copiedAccessPoint = *it;
        m_copiedBarrierElement = false;
    }
    m_hasCopiedElement = true;
    m_pasteAction->setEnabled(true);
    statusBar()->showMessage(tr("Copied element"), 3000);
}

void MainWindow::pasteElement()
{
    if (!m_hasCopiedElement)
        return;

    const int minX = m_copiedBarrierElement
                         ? std::min(m_copiedBarrier.x1, m_copiedBarrier.x2) : m_copiedAccessPoint.x;
    const int maxX = m_copiedBarrierElement
                         ? std::max(m_copiedBarrier.x1, m_copiedBarrier.x2) : m_copiedAccessPoint.x;
    const int minY = m_copiedBarrierElement
                         ? std::min(m_copiedBarrier.y1, m_copiedBarrier.y2) : m_copiedAccessPoint.y;
    const int maxY = m_copiedBarrierElement
                         ? std::max(m_copiedBarrier.y1, m_copiedBarrier.y2) : m_copiedAccessPoint.y;
    const auto preferredOffset = [](int minimum, int maximum, int limit) {
        if (maximum + 5 < limit)
            return 5;
        if (minimum - 5 >= 0)
            return -5;
        return 0;
    };
    const int preferredX = preferredOffset(minX, maxX, m_model.length());
    const int preferredY = preferredOffset(minY, maxY, m_model.width());
    if (preferredX == 0 || preferredY == 0) {
        reportError(tr("There is not enough room to paste the element 5 cells away."));
        return;
    }

    const QVector<QPoint> offsets = {
        {preferredX, preferredY}, {-preferredX, preferredY},
        {preferredX, -preferredY}, {-preferredX, -preferredY}
    };
    commitChange(m_copiedBarrierElement ? tr("Paste barrier") : tr("Paste access point"), [this, offsets] {
        for (const QPoint &offset : offsets) {
            const int x1 = m_copiedBarrierElement ? m_copiedBarrier.x1 + offset.x()
                                                   : m_copiedAccessPoint.x + offset.x();
            const int y1 = m_copiedBarrierElement ? m_copiedBarrier.y1 + offset.y()
                                                   : m_copiedAccessPoint.y + offset.y();
            const int x2 = m_copiedBarrierElement ? m_copiedBarrier.x2 + offset.x() : x1;
            const int y2 = m_copiedBarrierElement ? m_copiedBarrier.y2 + offset.y() : y1;
            const int candidateMinX = std::min(x1, x2);
            const int candidateMaxX = std::max(x1, x2);
            const int candidateMinY = std::min(y1, y2);
            const int candidateMaxY = std::max(y1, y2);
            if (candidateMinX < 0 || candidateMinY < 0 ||
                candidateMaxX >= m_model.length() || candidateMaxY >= m_model.width())
                continue;

            try {
                if (m_copiedBarrierElement) {
                    m_selectedId = m_model.addBarrier(x1, y1, x2, y2, m_copiedBarrier.thicknessCm,
                                                       m_copiedBarrier.material);
                    m_selectedBarrier = true;
                } else {
                    m_selectedId = m_model.addAccessPoint(x1, y1, m_copiedAccessPoint.frequencyGHz,
                                                          m_copiedAccessPoint.powerDbm);
                    m_selectedBarrier = false;
                }
                return;
            } catch (const std::invalid_argument &) {
            }
        }
        throw std::invalid_argument("No valid location is available to paste the element.");
    });
}

void MainWindow::finishDrag()
{
    if (!m_dragActive)
        return;

    m_dragActive = false;
    const QJsonObject after = m_model.toJson();
    if (after == m_dragBefore)
        return;

    m_model.loadJson(m_dragBefore);
    m_undoStack->push(new SnapshotCommand(m_model, m_dragBefore, after,
                                          [this] { refreshCanvas(); }, m_dragLabel));
    statusBar()->showMessage(m_dragLabel, 3000);
}

bool MainWindow::saveMap()
{
    if (m_currentFile.isEmpty()) {
        return saveMapAs();
    }
    QSaveFile file(m_currentFile);
    if (!file.open(QIODevice::WriteOnly)) {
        reportError(tr("Could not save map: %1").arg(file.errorString()));
        return false;
    }
    const QByteArray data = QJsonDocument(m_model.toJson()).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size() || !file.commit()) {
        reportError(tr("Could not save map: %1").arg(file.errorString()));
        return false;
    }
    statusBar()->showMessage(tr("Saved %1").arg(m_currentFile), 5000);
    return true;
}

bool MainWindow::saveMapAs()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Save map"), m_currentFile,
                                                       tr("WifiMap JSON (*.json)"));
    if (path.isEmpty())
        return false;
    m_currentFile = path.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive) ? path : path + QStringLiteral(".json");
    return saveMap();
}

void MainWindow::openMap()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Open map"), QString(),
                                                       tr("WifiMap JSON (*.json);;All files (*)"));
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        reportError(tr("Could not open map: %1").arg(file.errorString()));
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        reportError(tr("Invalid JSON map: %1").arg(parseError.errorString()));
        return;
    }
    const QJsonObject before = m_model.toJson();
    try {
        m_model.loadJson(document.object());
    } catch (const std::exception &error) {
        m_model.loadJson(before);
        reportError(QString::fromUtf8(error.what()));
        return;
    }
    m_currentFile = path;
    m_mapLength->setValue(m_model.length());
    m_mapWidth->setValue(m_model.width());
    m_undoStack->clear();
    selectElement(-1);
    refreshCanvas();
    m_canvas->fitToView();
    statusBar()->showMessage(tr("Loaded %1").arg(path), 5000);
}

void MainWindow::createNewMap()
{
    if (!m_model.accessPoints().isEmpty() || !m_model.barriers().isEmpty()) {
        const auto answer = QMessageBox::question(
            this, tr("New map"), tr("The current map contains elements. Save before creating a new map?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
        if (answer == QMessageBox::Cancel)
            return;
        if (answer == QMessageBox::Save) {
            if (!saveMap())
                return;
        }
    }
    commitChange(tr("Create new map"), [this] {
        m_model.clear(m_mapLength->value(), m_mapWidth->value());
        m_selectedId = -1;
    });
    m_currentFile.clear();
    selectElement(-1);
    m_canvas->fitToView();
}

void MainWindow::exportPng()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("Export map as PNG"), QString(),
                                                       tr("PNG image (*.png)"));
    if (path.isEmpty())
        return;
    const QString output = path.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)
                               ? path : path + QStringLiteral(".png");
    try {
        m_canvas->renderToFile(output);
        statusBar()->showMessage(tr("Exported %1").arg(output), 5000);
    } catch (const std::exception &error) {
        reportError(QString::fromUtf8(error.what()));
    }
}
