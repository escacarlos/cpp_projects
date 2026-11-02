#pragma once

#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QVector>

struct AccessPoint {
    int id = 0;
    int x = 0;
    int y = 0;
    double frequencyGHz = 2.4;
    int powerDbm = 11;
};

enum class WallMaterial {
    Plaster,
    Wood,
    Brick,
    Cement,
    ReinforcedConcrete,
    Metal
};

struct Barrier {
    int id = 0;
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    int thicknessCm = 1;
    WallMaterial material = WallMaterial::Plaster;
};

struct Parcel {
    double signalDbm = -100.0;
    QColor color;
};

class MapModel {
public:
    static constexpr int minimumDimension = 1;
    static constexpr int maximumDimension = 500;

    explicit MapModel(int length = 500, int width = 500);

    int length() const noexcept { return m_length; }
    int width() const noexcept { return m_width; }
    const QVector<AccessPoint> &accessPoints() const noexcept { return m_accessPoints; }
    const QVector<Barrier> &barriers() const noexcept { return m_barriers; }
    const Parcel &parcelAt(int x, int y) const;

    int addAccessPoint(int x, int y, double frequencyGHz, int powerDbm);
    void updateAccessPoint(int id, int x, int y, double frequencyGHz, int powerDbm);
    void moveAccessPoint(int id, int x, int y);
    void removeAccessPoint(int id);

    int addBarrier(int x1, int y1, int x2, int y2, int thicknessCm, WallMaterial material);
    void updateBarrier(int id, int x1, int y1, int x2, int y2, int thicknessCm, WallMaterial material);
    void moveBarrier(int id, int x1, int y1, int x2, int y2);
    void removeBarrier(int id);

    void resize(int length, int width);
    void clear(int length, int width);
    int accessPointAt(int x, int y, int radius = 2) const noexcept;
    int barrierAt(int x, int y, int radius = 2) const noexcept;

    QJsonObject toJson() const;
    void loadJson(const QJsonObject &json);
    QImage renderImage(int cellSize = 4, bool showGrid = false, int gridSize = 10) const;

    static QColor colorForSignal(double signalDbm);
    static QString materialName(WallMaterial material);
    static WallMaterial materialFromName(const QString &name);
    static double materialAttenuation(WallMaterial material);

private:
    int m_length;
    int m_width;
    int m_nextId = 1;
    QVector<AccessPoint> m_accessPoints;
    QVector<Barrier> m_barriers;
    QVector<Parcel> m_parcels;

    int parcelIndex(int x, int y) const;
    bool contains(int x, int y) const noexcept;
    AccessPoint &accessPointById(int id);
    Barrier &barrierById(int id);
    void validateDimensions(int length, int width) const;
    void validateAccessPoint(int x, int y, double frequencyGHz, int powerDbm, int ignoredId = -1) const;
    void validateBarrier(int x1, int y1, int x2, int y2, int thicknessCm, int ignoredId = -1) const;
    void recalculate();
    QImage heatmapImage() const;
};
