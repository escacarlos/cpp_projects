#pragma once

#include "domain/mapmodel.h"

#include <QWidget>

#include <functional>

class MapCanvas : public QWidget {
public:
    explicit MapCanvas(MapModel &model, QWidget *parent = nullptr);

    void refresh();
    void fitToView();
    void setGridVisible(bool visible);
    void setGridSize(int size);
    bool gridVisible() const noexcept { return m_gridVisible; }
    void setMagnetismEnabled(bool enabled) noexcept { m_magnetismEnabled = enabled; }
    void setBarrierDrawing(bool enabled);
    bool barrierDrawing() const noexcept { return m_barrierDrawing; }
    void setSelection(int id, bool barrier);
    void renderToFile(const QString &path) const;

    std::function<void(int, int)> cellClicked;
    std::function<void(int)> selectionChanged;
    std::function<void(int, int, int, int)> barrierDrawn;
    std::function<void(int, int, int)> accessPointMoved;
    std::function<void(int, int, int, int, int)> barrierMoved;
    std::function<void(int, bool)> dragStarted;
    std::function<void()> dragFinished;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    QSize sizeHint() const override;

private:
    MapModel &m_model;
    QImage m_mapImage;
    double m_scale = 1.0;
    QPointF m_offset;
    QPoint m_lastMouse;
    QPoint m_drawStart;
    QPoint m_drawCurrent;
    QPoint m_dragStart;
    int m_dragOriginX1 = 0;
    int m_dragOriginY1 = 0;
    int m_dragOriginX2 = 0;
    int m_dragOriginY2 = 0;
    int m_selectedId = -1;
    int m_draggingId = -1;
    bool m_selectedBarrier = false;
    bool m_dragging = false;
    bool m_panning = false;
    bool m_gridVisible = false;
    bool m_magnetismEnabled = false;
    bool m_barrierDrawing = false;
    bool m_hasDrawStart = false;
    int m_gridSize = 10;

    QPoint mapPoint(const QPointF &position, bool clamp) const;
    QPoint snapPoint(QPoint point) const;
    void updateDraggedElement(QPoint point);
    void drawScene(QPainter &painter, double cellSize, bool includeGrid) const;
    void drawAxes(QPainter &painter) const;
};
