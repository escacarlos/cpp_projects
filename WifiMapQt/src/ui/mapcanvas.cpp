#include "mapcanvas.h"

#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

MapCanvas::MapCanvas(MapModel &model, QWidget *parent)
    : QWidget(parent), m_model(model)
{
    setMinimumSize(420, 320);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setToolTipDuration(4000);
    refresh();
}

QSize MapCanvas::sizeHint() const
{
    return {900, 700};
}

void MapCanvas::refresh()
{
    m_mapImage = QImage(m_model.length(), m_model.width(), QImage::Format_RGB32);
    for (int y = 0; y < m_model.width(); ++y) {
        for (int x = 0; x < m_model.length(); ++x) {
            const QColor color = m_model.parcelAt(x, y).color;
            m_mapImage.setPixelColor(x, m_model.width() - 1 - y,
                                     qRgb(color.red(), color.green(), color.blue()));
        }
    }
    update();
}

void MapCanvas::fitToView()
{
    constexpr int leftAxisWidth = 44;
    constexpr int bottomAxisHeight = 34;
    const double usableWidth = std::max(1, width() - leftAxisWidth - 14);
    const double usableHeight = std::max(1, height() - bottomAxisHeight - 14);
    m_scale = std::max(0.1, std::min(usableWidth / m_model.length(),
                                    usableHeight / m_model.width()));
    m_offset = QPointF(leftAxisWidth + (usableWidth - m_model.length() * m_scale) / 2.0,
                       (height() - bottomAxisHeight - m_model.width() * m_scale) / 2.0);
    update();
}

void MapCanvas::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    update();
}

void MapCanvas::setGridSize(int size)
{
    m_gridSize = std::clamp(size, 1, 50);
    update();
}

void MapCanvas::setBarrierDrawing(bool enabled)
{
    m_barrierDrawing = enabled;
    m_drawStart = {};
    m_drawCurrent = {};
    m_hasDrawStart = false;
    setCursor(enabled ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

void MapCanvas::setSelection(int id, bool barrier)
{
    m_selectedId = id;
    m_selectedBarrier = barrier;
    update();
}

QPoint MapCanvas::mapPoint(const QPointF &position, bool clamp) const
{
    const double x = (position.x() - m_offset.x()) / m_scale;
    const double row = (position.y() - m_offset.y()) / m_scale;
    int mapX = static_cast<int>(std::floor(x));
    int mapY = m_model.width() - 1 - static_cast<int>(std::floor(row));
    if (clamp) {
        mapX = std::clamp(mapX, 0, m_model.length() - 1);
        mapY = std::clamp(mapY, 0, m_model.width() - 1);
    }
    return {mapX, mapY};
}

QPoint MapCanvas::snapPoint(QPoint point) const
{
    if (!m_magnetismEnabled)
        return point;
    const int grid = m_gridSize;
    point.setX(std::clamp(static_cast<int>(std::round(point.x() / static_cast<double>(grid))) * grid,
                          0, m_model.length() - 1));
    point.setY(std::clamp(static_cast<int>(std::round(point.y() / static_cast<double>(grid))) * grid,
                          0, m_model.width() - 1));
    return point;
}

void MapCanvas::updateDraggedElement(QPoint point)
{
    if (!m_dragging)
        return;

    point = snapPoint(point);
    const int dx = point.x() - m_dragStart.x();
    const int dy = point.y() - m_dragStart.y();
    if (m_selectedBarrier) {
        const auto it = std::find_if(m_model.barriers().cbegin(), m_model.barriers().cend(),
                                     [this](const Barrier &barrier) { return barrier.id == m_draggingId; });
        if (it != m_model.barriers().cend()) {
            const int x1 = m_dragOriginX1 + dx;
            const int y1 = m_dragOriginY1 + dy;
            const int x2 = m_dragOriginX2 + dx;
            const int y2 = m_dragOriginY2 + dy;
            if ((it->x1 != x1 || it->y1 != y1 || it->x2 != x2 || it->y2 != y2) && barrierMoved)
                barrierMoved(m_draggingId, x1, y1, x2, y2);
        }
    } else {
        const auto it = std::find_if(m_model.accessPoints().cbegin(), m_model.accessPoints().cend(),
                                     [this](const AccessPoint &accessPoint) {
                                         return accessPoint.id == m_draggingId;
                                     });
        if (it != m_model.accessPoints().cend() &&
            (it->x != point.x() || it->y != point.y()) && accessPointMoved)
            accessPointMoved(m_draggingId, point.x(), point.y());
    }
}

void MapCanvas::drawScene(QPainter &painter, double cellSize, bool includeGrid) const
{
    painter.setRenderHint(QPainter::SmoothPixmapTransform, cellSize < 1.0);
    const QRectF mapRect(m_offset, QSizeF(m_model.length() * cellSize, m_model.width() * cellSize));
    painter.fillRect(mapRect, Qt::white);
    if (!m_mapImage.isNull())
        painter.drawImage(mapRect, m_mapImage);

    for (const Barrier &barrier : m_model.barriers()) {
        QColor color;
        switch (barrier.material) {
        case WallMaterial::Plaster: color = QColor(255, 204, 204); break;
        case WallMaterial::Wood: color = QColor(153, 102, 51); break;
        case WallMaterial::Brick: color = QColor(204, 102, 0); break;
        case WallMaterial::Cement: color = QColor(160, 160, 160); break;
        case WallMaterial::ReinforcedConcrete: color = QColor(100, 100, 100); break;
        case WallMaterial::Metal: color = QColor(0, 128, 192); break;
        }
        const int x = std::min(barrier.x1, barrier.x2);
        const int y = std::max(barrier.y1, barrier.y2);
        const double left = m_offset.x() + x * cellSize;
        const double top = m_offset.y() + (m_model.width() - 1 - y) * cellSize;
        const double thickness = std::clamp(cellSize * barrier.thicknessCm / 60.0,
                                             std::min(1.0, cellSize), cellSize);
        const QRectF rect(left, top,
                          barrier.x1 == barrier.x2
                              ? thickness : (std::abs(barrier.x2 - barrier.x1) + 1) * cellSize,
                          barrier.y1 == barrier.y2
                              ? thickness : (std::abs(barrier.y2 - barrier.y1) + 1) * cellSize);
        painter.setPen(m_selectedBarrier && barrier.id == m_selectedId
                           ? QPen(Qt::red, std::max(1.0, cellSize * 0.12))
                           : Qt::NoPen);
        painter.setBrush(color);
        painter.drawRect(rect);
    }

    for (const AccessPoint &point : m_model.accessPoints()) {
        const QPointF center(m_offset.x() + (point.x + 0.5) * cellSize,
                             m_offset.y() + (m_model.width() - point.y - 0.5) * cellSize);
        const double radius = std::max(2.0, cellSize * 0.38);
        painter.setPen(QPen(point.id == m_selectedId && !m_selectedBarrier ? Qt::red : Qt::black,
                            std::max(1.0, cellSize * 0.1)));
        painter.setBrush(Qt::white);
        painter.drawEllipse(center, radius, radius);
        painter.setBrush(Qt::black);
        painter.drawEllipse(center, std::max(1.0, radius * 0.38), std::max(1.0, radius * 0.38));
    }

    if (includeGrid && cellSize >= 3.0) {
        painter.setPen(QPen(QColor(40, 40, 40, 45), 1));
        for (int x = 0; x <= m_model.length(); x += m_gridSize) {
            const double px = m_offset.x() + x * cellSize;
            painter.drawLine(QPointF(px, m_offset.y()),
                             QPointF(px, m_offset.y() + m_model.width() * cellSize));
        }
        for (int y = 0; y <= m_model.width(); y += m_gridSize) {
            const double py = m_offset.y() + y * cellSize;
            painter.drawLine(QPointF(m_offset.x(), py),
                             QPointF(m_offset.x() + m_model.length() * cellSize, py));
        }
    }

    if (m_barrierDrawing && m_hasDrawStart) {
        painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
        const QPoint first(m_offset.x() + (m_drawStart.x() + 0.5) * cellSize,
                           m_offset.y() + (m_model.width() - m_drawStart.y() - 0.5) * cellSize);
        const QPoint second(m_offset.x() + (m_drawCurrent.x() + 0.5) * cellSize,
                            m_offset.y() + (m_model.width() - m_drawCurrent.y() - 0.5) * cellSize);
        if (std::abs(second.x() - first.x()) >= std::abs(second.y() - first.y()))
            painter.drawLine(first, QPoint(second.x(), first.y()));
        else
            painter.drawLine(first, QPoint(first.x(), second.y()));
    }
}

void MapCanvas::drawAxes(QPainter &painter) const
{
    const double mapWidth = m_model.length() * m_scale;
    const double mapHeight = m_model.width() * m_scale;
    const double left = m_offset.x();
    const double right = left + mapWidth;
    const double top = m_offset.y();
    const double bottom = top + mapHeight;
    const int horizontalStep = m_gridSize *
        std::max(1, static_cast<int>(std::ceil(56.0 / (m_scale * m_gridSize))));
    const int verticalStep = horizontalStep;

    painter.save();
    painter.setPen(QPen(QColor(70, 70, 70), 1));
    painter.drawLine(QPointF(left, bottom), QPointF(right, bottom));
    painter.drawLine(QPointF(left, top), QPointF(left, bottom));

    const QFontMetrics metrics(painter.font());
    const int labelHeight = metrics.height();
    for (int x = 0; x < m_model.length(); x += horizontalStep) {
        const double px = left + (x + 0.5) * m_scale;
        painter.drawLine(QPointF(px, bottom), QPointF(px, bottom + 4));
        const QString label = QString::number(x);
        const int labelWidth = metrics.horizontalAdvance(label);
        painter.drawText(QRectF(px - labelWidth / 2.0, bottom + 5,
                                labelWidth, labelHeight),
                         Qt::AlignHCenter | Qt::AlignTop, label);
    }

    for (int y = 0; y < m_model.width(); y += verticalStep) {
        const double py = bottom - (y + 0.5) * m_scale;
        painter.drawLine(QPointF(left - 4, py), QPointF(left, py));
        painter.drawText(QRectF(0, py - labelHeight / 2.0, left - 7, labelHeight),
                         Qt::AlignRight | Qt::AlignVCenter, QString::number(y));
    }

    painter.drawText(QRectF(right - 18, bottom + 18, 18, labelHeight),
                     Qt::AlignRight | Qt::AlignTop, tr("X"));
    painter.save();
    painter.translate(12, top + mapHeight / 2.0);
    painter.rotate(-90);
    painter.drawText(QRectF(-mapHeight / 2.0, -labelHeight / 2.0, mapHeight, labelHeight),
                     Qt::AlignHCenter | Qt::AlignVCenter, tr("Y"));
    painter.restore();
    painter.restore();
}

void MapCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(238, 240, 243));
    painter.save();
    drawScene(painter, m_scale, m_gridVisible);
    painter.restore();
    drawAxes(painter);

    const QRect legendRect(width() - 230, height() - 54, 214, 40);
    painter.setPen(QPen(QColor(70, 70, 70), 1));
    painter.setBrush(QColor(255, 255, 255, 235));
    painter.drawRoundedRect(legendRect, 4, 4);
    painter.setPen(Qt::black);
    painter.drawText(legendRect.adjusted(8, 3, -8, -20), Qt::AlignLeft | Qt::AlignVCenter,
                     tr("Signal strength (dBm)"));
    const QRect gradientRect(legendRect.x() + 8, legendRect.y() + 23, 164, 10);
    for (int x = 0; x < gradientRect.width(); ++x) {
        const double signal = -100.0 + 80.0 * x / std::max(1, gradientRect.width() - 1);
        painter.setPen(MapModel::colorForSignal(signal));
        painter.drawLine(gradientRect.x() + x, gradientRect.y(),
                         gradientRect.x() + x, gradientRect.bottom());
    }
    painter.setPen(Qt::black);
    painter.drawText(QRect(legendRect.x() + 176, legendRect.y() + 19, 34, 16),
                     Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("-20"));
    painter.drawText(QRect(legendRect.x() + 8, legendRect.y() + 32, 34, 12),
                     Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("-100"));
}

void MapCanvas::mousePressEvent(QMouseEvent *event)
{
    m_lastMouse = event->pos();
    setFocus();
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        m_panning = true;
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    if (event->button() != Qt::LeftButton)
        return;

    const QPoint point = mapPoint(event->position(), false);
    if (m_barrierDrawing) {
        if (point.x() >= 0 && point.x() < m_model.length() &&
            point.y() >= 0 && point.y() < m_model.width()) {
            m_drawStart = point;
            m_drawCurrent = point;
            m_hasDrawStart = true;
        }
        return;
    }

    if (point.x() < 0 || point.x() >= m_model.length() ||
        point.y() < 0 || point.y() >= m_model.width())
        return;
    if (cellClicked)
        cellClicked(point.x(), point.y());

    const int pointId = m_model.accessPointAt(point.x(), point.y(), 1);
    const int barrierId = m_model.barrierAt(point.x(), point.y(), 1);
    if (pointId >= 0) {
        m_selectedId = pointId;
        m_selectedBarrier = false;
        m_draggingId = pointId;
        m_dragging = true;
        m_dragStart = point;
        const auto it = std::find_if(m_model.accessPoints().cbegin(), m_model.accessPoints().cend(),
                                     [pointId](const AccessPoint &accessPoint) {
                                         return accessPoint.id == pointId;
                                     });
        if (it != m_model.accessPoints().cend()) {
            m_dragOriginX1 = it->x;
            m_dragOriginY1 = it->y;
        }
    } else if (barrierId >= 0) {
        m_selectedId = barrierId;
        m_selectedBarrier = true;
        m_draggingId = barrierId;
        m_dragging = true;
        m_dragStart = point;
        const auto it = std::find_if(m_model.barriers().cbegin(), m_model.barriers().cend(),
                                     [barrierId](const Barrier &barrier) { return barrier.id == barrierId; });
        if (it != m_model.barriers().cend()) {
            m_dragOriginX1 = it->x1;
            m_dragOriginY1 = it->y1;
            m_dragOriginX2 = it->x2;
            m_dragOriginY2 = it->y2;
        }
    } else {
        m_selectedId = -1;
        m_draggingId = -1;
        m_dragging = false;
    }
    if (selectionChanged)
        selectionChanged(m_selectedId);
    if (m_dragging && dragStarted)
        dragStarted(m_draggingId, m_selectedBarrier);
    update();
}

void MapCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        const QPoint delta = event->pos() - m_lastMouse;
        m_offset += delta;
        m_lastMouse = event->pos();
        update();
        return;
    }

    const QPoint point = mapPoint(event->position(), false);
    if (m_barrierDrawing && m_hasDrawStart && (event->buttons() & Qt::LeftButton)) {
        m_drawCurrent = mapPoint(event->position(), true);
        update();
        return;
    }

    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        updateDraggedElement(mapPoint(event->position(), true));
        return;
    }

    if (point.x() >= 0 && point.x() < m_model.length() &&
        point.y() >= 0 && point.y() < m_model.width()) {
        const int pointId = m_model.accessPointAt(point.x(), point.y(), 1);
        const int barrierId = m_model.barrierAt(point.x(), point.y(), 1);
        if (pointId >= 0) {
            const auto it = std::find_if(m_model.accessPoints().cbegin(), m_model.accessPoints().cend(),
                                         [pointId](const AccessPoint &p) { return p.id == pointId; });
            if (it != m_model.accessPoints().cend())
                QToolTip::showText(event->globalPosition().toPoint(),
                    tr("Access point\nCoordinates: (%1, %2)\nFrequency: %3 GHz\nPower: %4 dBm")
                        .arg(it->x).arg(it->y).arg(it->frequencyGHz, 0, 'f', 1).arg(it->powerDbm), this);
        } else if (barrierId >= 0) {
            const auto it = std::find_if(m_model.barriers().cbegin(), m_model.barriers().cend(),
                                         [barrierId](const Barrier &b) { return b.id == barrierId; });
            if (it != m_model.barriers().cend())
                QToolTip::showText(event->globalPosition().toPoint(),
                    tr("Barrier\nMaterial: %1\nThickness: %2 cm\nAttenuation: %3")
                        .arg(MapModel::materialName(it->material)).arg(it->thicknessCm)
                        .arg(MapModel::materialAttenuation(it->material), 0, 'f', 2), this);
        } else {
            const Parcel &parcel = m_model.parcelAt(point.x(), point.y());
            QString quality;
            if (parcel.signalDbm >= -60.0)
                quality = tr("Excellent");
            else if (parcel.signalDbm >= -70.0)
                quality = tr("Good");
            else if (parcel.signalDbm >= -80.0)
                quality = tr("Weak");
            else if (parcel.signalDbm >= -90.0)
                quality = tr("Very weak");
            else
                quality = tr("Out of range");
            QToolTip::showText(event->globalPosition().toPoint(),
                tr("Parcel (%1, %2)\nSignal: %3 dBm\nQuality: %4")
                    .arg(point.x()).arg(point.y()).arg(parcel.signalDbm, 0, 'f', 2).arg(quality), this);
        }
    }
}

void MapCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        m_panning = false;
        setCursor(m_barrierDrawing ? Qt::CrossCursor : Qt::ArrowCursor);
        return;
    }
    if (event->button() != Qt::LeftButton)
        return;

    if (m_barrierDrawing && m_hasDrawStart) {
        QPoint end = mapPoint(event->position(), true);
        QPoint start = m_drawStart;
        if (m_magnetismEnabled) {
                const int grid = m_gridSize;
            start.setX(std::clamp(static_cast<int>(std::round(start.x() / static_cast<double>(grid))) * grid,
                                  0, m_model.length() - 1));
            start.setY(std::clamp(static_cast<int>(std::round(start.y() / static_cast<double>(grid))) * grid,
                                  0, m_model.width() - 1));
            end.setX(std::clamp(static_cast<int>(std::round(end.x() / static_cast<double>(grid))) * grid,
                                0, m_model.length() - 1));
            end.setY(std::clamp(static_cast<int>(std::round(end.y() / static_cast<double>(grid))) * grid,
                                0, m_model.width() - 1));
        }
        if (std::abs(end.x() - start.x()) >= std::abs(end.y() - start.y()))
            end.setY(start.y());
        else
            end.setX(start.x());
        if (barrierDrawn)
            barrierDrawn(start.x(), start.y(), end.x(), end.y());
        m_drawStart = {};
        m_drawCurrent = {};
        m_hasDrawStart = false;
        update();
        return;
    }
    if (m_dragging) {
        updateDraggedElement(mapPoint(event->position(), true));
    }
    const bool finishedDrag = m_dragging;
    m_dragging = false;
    m_draggingId = -1;
    if (finishedDrag && dragFinished)
        dragFinished();
}

void MapCanvas::wheelEvent(QWheelEvent *event)
{
    const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    const QPointF cursor = event->position();
    const QPointF mapPosition = (cursor - m_offset) / m_scale;
    m_scale = std::clamp(m_scale * factor, 0.15, 80.0);
    m_offset = cursor - mapPosition * m_scale;
    update();
}

void MapCanvas::renderToFile(const QString &path) const
{
    const QImage image = m_model.renderImage(4, m_gridVisible, m_gridSize);
    if (!image.save(path, "PNG"))
        throw std::runtime_error("Could not write PNG image");
}
