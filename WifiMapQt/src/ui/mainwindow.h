#pragma once

#include "domain/mapmodel.h"

#include <QMainWindow>

#include <functional>

class QComboBox;
class QLabel;
class QSpinBox;
class QPushButton;
class QUndoStack;
class MapCanvas;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    MapModel m_model;
    MapCanvas *m_canvas = nullptr;
    QUndoStack *m_undoStack = nullptr;
    QSpinBox *m_pointX = nullptr;
    QSpinBox *m_pointY = nullptr;
    QComboBox *m_frequency = nullptr;
    QSpinBox *m_power = nullptr;
    QPushButton *m_addPoint = nullptr;
    QPushButton *m_updatePoint = nullptr;
    QPushButton *m_deletePoint = nullptr;
    QSpinBox *m_x1 = nullptr;
    QSpinBox *m_y1 = nullptr;
    QSpinBox *m_x2 = nullptr;
    QSpinBox *m_y2 = nullptr;
    QSpinBox *m_thickness = nullptr;
    QComboBox *m_material = nullptr;
    QPushButton *m_addBarrier = nullptr;
    QPushButton *m_updateBarrier = nullptr;
    QPushButton *m_deleteBarrier = nullptr;
    QSpinBox *m_mapLength = nullptr;
    QSpinBox *m_mapWidth = nullptr;
    QLabel *m_selectionLabel = nullptr;
    int m_selectedId = -1;
    bool m_selectedBarrier = false;
    QString m_currentFile;

    void createUi();
    void createMenus();
    void selectElement(int id);
    void refreshCanvas();
    void reportError(const QString &message);
    void commitChange(const QString &label, const std::function<void()> &change);
    bool saveMap();
    bool saveMapAs();
    void openMap();
    void createNewMap();
    void exportPng();
};
