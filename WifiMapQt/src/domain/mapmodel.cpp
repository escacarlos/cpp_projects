#include "mapmodel.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QPainter>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr double pi = 3.14159265358979323846;

struct SignalLevel {
    double dbm;
    QColor color;
};

const SignalLevel levels[] = {
    {-20.0, QColor(147, 0, 0)}, {-30.0, QColor(255, 5, 2)},
    {-40.0, QColor(254, 130, 0)}, {-50.0, QColor(254, 252, 0)},
    {-60.0, QColor(0, 255, 0)}, {-70.0, QColor(0, 255, 255)},
    {-80.0, QColor(0, 125, 255)}, {-90.0, QColor(0, 0, 255)},
    {-100.0, QColor(0, 0, 100)}
};

bool segmentsIntersect(int ax, int ay, int bx, int by, const Barrier &barrier)
{
    if (barrier.y1 == barrier.y2) {
        const int lowX = std::min(barrier.x1, barrier.x2);
        const int highX = std::max(barrier.x1, barrier.x2);
        if (ay == by)
            return ay == barrier.y1 && std::max(std::min(ax, bx), lowX) <= std::min(std::max(ax, bx), highX);
        const double t = static_cast<double>(barrier.y1 - ay) / (by - ay);
        const double x = ax + t * (bx - ax);
        return t >= 0.0 && t <= 1.0 && x >= lowX && x <= highX;
    }

    const int lowY = std::min(barrier.y1, barrier.y2);
    const int highY = std::max(barrier.y1, barrier.y2);
    if (ax == bx)
        return ax == barrier.x1 && std::max(std::min(ay, by), lowY) <= std::min(std::max(ay, by), highY);
    const double t = static_cast<double>(barrier.x1 - ax) / (bx - ax);
    const double y = ay + t * (by - ay);
    return t >= 0.0 && t <= 1.0 && y >= lowY && y <= highY;
}

int readInteger(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (!value.isDouble() || std::floor(value.toDouble()) != value.toDouble())
        throw std::invalid_argument(QStringLiteral("Invalid integer field: %1").arg(key).toStdString());
    return value.toInt();
}
}

MapModel::MapModel(int length, int width)
    : m_length(length), m_width(width)
{
    validateDimensions(length, width);
    m_parcels.resize(length * width);
    recalculate();
}

const Parcel &MapModel::parcelAt(int x, int y) const
{
    if (!contains(x, y))
        throw std::out_of_range("Parcel coordinates are outside the map");
    return m_parcels.at(parcelIndex(x, y));
}

int MapModel::parcelIndex(int x, int y) const
{
    return y * m_length + x;
}

bool MapModel::contains(int x, int y) const noexcept
{
    return x >= 0 && x < m_length && y >= 0 && y < m_width;
}

void MapModel::validateDimensions(int length, int width) const
{
    if (length < minimumDimension || length > maximumDimension ||
        width < minimumDimension || width > maximumDimension)
        throw std::invalid_argument("Map dimensions must be between 1 and 500");
}

void MapModel::validateAccessPoint(int x, int y, double frequencyGHz, int powerDbm, int ignoredId) const
{
    if (!contains(x, y))
        throw std::invalid_argument("Access point is outside the map");
    if (frequencyGHz != 2.4 && frequencyGHz != 5.0)
        throw std::invalid_argument("Frequency must be 2.4 or 5.0 GHz");
    if (powerDbm < 11 || powerDbm > 20)
        throw std::invalid_argument("Transmitted power must be between 11 and 20 dBm");

    for (const AccessPoint &point : m_accessPoints) {
        if (point.id != ignoredId && point.x == x && point.y == y)
            throw std::invalid_argument("An access point already occupies this cell");
    }
    for (const Barrier &barrier : m_barriers) {
        if ((barrier.x1 == barrier.x2 && x == barrier.x1 &&
             y >= std::min(barrier.y1, barrier.y2) && y <= std::max(barrier.y1, barrier.y2)) ||
            (barrier.y1 == barrier.y2 && y == barrier.y1 &&
             x >= std::min(barrier.x1, barrier.x2) && x <= std::max(barrier.x1, barrier.x2)))
            throw std::invalid_argument("An access point cannot occupy a barrier");
    }
}

void MapModel::validateBarrier(int x1, int y1, int x2, int y2, int thicknessCm, int ignoredId) const
{
    if (!contains(x1, y1) || !contains(x2, y2))
        throw std::invalid_argument("Barrier endpoints must be inside the map");
    if (x1 != x2 && y1 != y2)
        throw std::invalid_argument("Barriers must be horizontal or vertical");
    if (thicknessCm < 1 || thicknessCm > 499)
        throw std::invalid_argument("Barrier thickness must be between 1 and 499 cm");

    for (const AccessPoint &point : m_accessPoints) {
        const bool onVertical = x1 == x2 && point.x == x1 &&
            point.y >= std::min(y1, y2) && point.y <= std::max(y1, y2);
        const bool onHorizontal = y1 == y2 && point.y == y1 &&
            point.x >= std::min(x1, x2) && point.x <= std::max(x1, x2);
        if (point.id != ignoredId && (onVertical || onHorizontal))
            throw std::invalid_argument("A barrier cannot intersect an access point");
    }
}

AccessPoint &MapModel::accessPointById(int id)
{
    const auto it = std::find_if(m_accessPoints.begin(), m_accessPoints.end(),
                                 [id](const AccessPoint &point) { return point.id == id; });
    if (it == m_accessPoints.end())
        throw std::invalid_argument("Access point was not found");
    return *it;
}

Barrier &MapModel::barrierById(int id)
{
    const auto it = std::find_if(m_barriers.begin(), m_barriers.end(),
                                 [id](const Barrier &barrier) { return barrier.id == id; });
    if (it == m_barriers.end())
        throw std::invalid_argument("Barrier was not found");
    return *it;
}

int MapModel::addAccessPoint(int x, int y, double frequencyGHz, int powerDbm)
{
    validateAccessPoint(x, y, frequencyGHz, powerDbm);
    const int id = m_nextId++;
    m_accessPoints.push_back({id, x, y, frequencyGHz, powerDbm});
    recalculate();
    return id;
}

void MapModel::updateAccessPoint(int id, int x, int y, double frequencyGHz, int powerDbm)
{
    validateAccessPoint(x, y, frequencyGHz, powerDbm, id);
    accessPointById(id) = {id, x, y, frequencyGHz, powerDbm};
    recalculate();
}

void MapModel::moveAccessPoint(int id, int x, int y)
{
    const AccessPoint old = accessPointById(id);
    updateAccessPoint(id, x, y, old.frequencyGHz, old.powerDbm);
}

void MapModel::removeAccessPoint(int id)
{
    accessPointById(id);
    m_accessPoints.erase(std::remove_if(m_accessPoints.begin(), m_accessPoints.end(),
                                       [id](const AccessPoint &point) { return point.id == id; }),
                         m_accessPoints.end());
    recalculate();
}

int MapModel::addBarrier(int x1, int y1, int x2, int y2, int thicknessCm, WallMaterial material)
{
    validateBarrier(x1, y1, x2, y2, thicknessCm);
    const int id = m_nextId++;
    m_barriers.push_back({id, x1, y1, x2, y2, thicknessCm, material});
    recalculate();
    return id;
}

void MapModel::updateBarrier(int id, int x1, int y1, int x2, int y2, int thicknessCm, WallMaterial material)
{
    validateBarrier(x1, y1, x2, y2, thicknessCm);
    barrierById(id) = {id, x1, y1, x2, y2, thicknessCm, material};
    recalculate();
}

void MapModel::moveBarrier(int id, int x1, int y1, int x2, int y2)
{
    const Barrier old = barrierById(id);
    updateBarrier(id, x1, y1, x2, y2, old.thicknessCm, old.material);
}

void MapModel::removeBarrier(int id)
{
    barrierById(id);
    m_barriers.erase(std::remove_if(m_barriers.begin(), m_barriers.end(),
                                   [id](const Barrier &barrier) { return barrier.id == id; }),
                     m_barriers.end());
    recalculate();
}

void MapModel::resize(int length, int width)
{
    validateDimensions(length, width);
    for (const AccessPoint &point : m_accessPoints) {
        if (point.x >= length || point.y >= width)
            throw std::invalid_argument("Resize would place an access point outside the map");
    }
    for (const Barrier &barrier : m_barriers) {
        if (barrier.x1 >= length || barrier.x2 >= length ||
            barrier.y1 >= width || barrier.y2 >= width)
            throw std::invalid_argument("Resize would place a barrier outside the map");
    }
    m_length = length;
    m_width = width;
    m_parcels.resize(length * width);
    recalculate();
}

void MapModel::clear(int length, int width)
{
    validateDimensions(length, width);
    m_length = length;
    m_width = width;
    m_accessPoints.clear();
    m_barriers.clear();
    m_nextId = 1;
    m_parcels.resize(length * width);
    recalculate();
}

int MapModel::accessPointAt(int x, int y, int radius) const noexcept
{
    for (const AccessPoint &point : m_accessPoints) {
        if (std::abs(point.x - x) <= radius && std::abs(point.y - y) <= radius)
            return point.id;
    }
    return -1;
}

int MapModel::barrierAt(int x, int y, int radius) const noexcept
{
    for (auto it = m_barriers.crbegin(); it != m_barriers.crend(); ++it) {
        const Barrier &barrier = *it;
        const int lowX = std::min(barrier.x1, barrier.x2);
        const int highX = std::max(barrier.x1, barrier.x2);
        const int lowY = std::min(barrier.y1, barrier.y2);
        const int highY = std::max(barrier.y1, barrier.y2);
        if (barrier.y1 == barrier.y2 &&
            x >= lowX - radius && x <= highX + radius &&
            y >= barrier.y1 - radius && y <= barrier.y1 + radius)
            return barrier.id;
        if (barrier.x1 == barrier.x2 &&
            x >= barrier.x1 - radius && x <= barrier.x1 + radius &&
            y >= lowY - radius && y <= highY + radius)
            return barrier.id;
    }
    return -1;
}

QColor MapModel::colorForSignal(double signalDbm)
{
    const double value = std::clamp(signalDbm, -100.0, -20.0);
    for (int i = 0; i < 8; ++i) {
        const double upper = levels[i].dbm;
        const double lower = levels[i + 1].dbm;
        if (value <= upper && value >= lower) {
            const double ratio = (value - lower) / (upper - lower);
            return QColor(
                static_cast<int>(levels[i + 1].color.red() +
                                 ratio * (levels[i].color.red() - levels[i + 1].color.red())),
                static_cast<int>(levels[i + 1].color.green() +
                                 ratio * (levels[i].color.green() - levels[i + 1].color.green())),
                static_cast<int>(levels[i + 1].color.blue() +
                                 ratio * (levels[i].color.blue() - levels[i + 1].color.blue())));
        }
    }
    return levels[8].color;
}

QString MapModel::materialName(WallMaterial material)
{
    switch (material) {
    case WallMaterial::Plaster: return QStringLiteral("Plaster");
    case WallMaterial::Wood: return QStringLiteral("Wood");
    case WallMaterial::Brick: return QStringLiteral("Brick");
    case WallMaterial::Cement: return QStringLiteral("Cement");
    case WallMaterial::ReinforcedConcrete: return QStringLiteral("Reinforced concrete");
    case WallMaterial::Metal: return QStringLiteral("Metal");
    }
    throw std::invalid_argument("Unknown barrier material");
}

WallMaterial MapModel::materialFromName(const QString &name)
{
    for (WallMaterial material : {WallMaterial::Plaster, WallMaterial::Wood, WallMaterial::Brick,
                                  WallMaterial::Cement, WallMaterial::ReinforcedConcrete, WallMaterial::Metal}) {
        if (materialName(material) == name)
            return material;
    }
    throw std::invalid_argument("Unknown barrier material");
}

double MapModel::materialAttenuation(WallMaterial material)
{
    switch (material) {
    case WallMaterial::Plaster: return 0.3;
    case WallMaterial::Wood: return 0.5;
    case WallMaterial::Brick: return 1.0;
    case WallMaterial::Cement: return 1.33;
    case WallMaterial::ReinforcedConcrete: return 1.66;
    case WallMaterial::Metal: return 2.0;
    }
    throw std::invalid_argument("Unknown barrier material");
}

void MapModel::recalculate()
{
    m_parcels.resize(m_length * m_width);
    for (int y = 0; y < m_width; ++y) {
        for (int x = 0; x < m_length; ++x) {
            double strongest = -100.0;
            for (const AccessPoint &point : m_accessPoints) {
                const double distance = std::hypot(x - point.x, y - point.y);
                const double wavelength = point.frequencyGHz == 2.4 ? 0.125 : 0.06;
                double signal = point.powerDbm +
                    20.0 * std::log10(wavelength / (4.0 * pi * std::max(distance, 1.0)));
                for (const Barrier &barrier : m_barriers) {
                    if (segmentsIntersect(point.x, point.y, x, y, barrier))
                        signal -= static_cast<int>(barrier.thicknessCm * materialAttenuation(barrier.material));
                }
                strongest = std::max(strongest, signal);
            }
            Parcel &parcel = m_parcels[parcelIndex(x, y)];
            parcel.signalDbm = strongest;
            parcel.color = colorForSignal(strongest);
        }
    }
}

QJsonObject MapModel::toJson() const
{
    QJsonArray points;
    for (const AccessPoint &point : m_accessPoints) {
        points.append(QJsonObject{
            {QStringLiteral("id"), point.id}, {QStringLiteral("x"), point.x},
            {QStringLiteral("y"), point.y}, {QStringLiteral("frequencyGHz"), point.frequencyGHz},
            {QStringLiteral("powerDbm"), point.powerDbm}
        });
    }

    QJsonArray barriers;
    for (const Barrier &barrier : m_barriers) {
        barriers.append(QJsonObject{
            {QStringLiteral("id"), barrier.id}, {QStringLiteral("x1"), barrier.x1},
            {QStringLiteral("y1"), barrier.y1}, {QStringLiteral("x2"), barrier.x2},
            {QStringLiteral("y2"), barrier.y2}, {QStringLiteral("thicknessCm"), barrier.thicknessCm},
            {QStringLiteral("material"), materialName(barrier.material)}
        });
    }

    return {
        {QStringLiteral("formatVersion"), 1},
        {QStringLiteral("length"), m_length},
        {QStringLiteral("width"), m_width},
        {QStringLiteral("accessPoints"), points},
        {QStringLiteral("barriers"), barriers}
    };
}

void MapModel::loadJson(const QJsonObject &json)
{
    if (readInteger(json, QStringLiteral("formatVersion")) != 1)
        throw std::invalid_argument("Unsupported map format version");
    const int length = readInteger(json, QStringLiteral("length"));
    const int width = readInteger(json, QStringLiteral("width"));
    validateDimensions(length, width);
    if (!json.value(QStringLiteral("accessPoints")).isArray() ||
        !json.value(QStringLiteral("barriers")).isArray())
        throw std::invalid_argument("Map file must contain accessPoints and barriers arrays");

    MapModel candidate(length, width);
    int largestId = 0;
    QVector<int> usedIds;
    for (const QJsonValue &value : json.value(QStringLiteral("accessPoints")).toArray()) {
        if (!value.isObject())
            throw std::invalid_argument("Invalid access point entry");
        const QJsonObject point = value.toObject();
        const int id = readInteger(point, QStringLiteral("id"));
        if (id <= 0 || usedIds.contains(id))
            throw std::invalid_argument("Invalid access point ID");
        usedIds.push_back(id);
        candidate.validateAccessPoint(readInteger(point, QStringLiteral("x")),
                                      readInteger(point, QStringLiteral("y")),
                                      point.value(QStringLiteral("frequencyGHz")).toDouble(-1.0),
                                      readInteger(point, QStringLiteral("powerDbm")));
        candidate.m_accessPoints.push_back({
            id, readInteger(point, QStringLiteral("x")), readInteger(point, QStringLiteral("y")),
            point.value(QStringLiteral("frequencyGHz")).toDouble(),
            readInteger(point, QStringLiteral("powerDbm"))
        });
        largestId = std::max(largestId, id);
    }
    for (const QJsonValue &value : json.value(QStringLiteral("barriers")).toArray()) {
        if (!value.isObject())
            throw std::invalid_argument("Invalid barrier entry");
        const QJsonObject wall = value.toObject();
        const int id = readInteger(wall, QStringLiteral("id"));
        if (id <= 0 || usedIds.contains(id))
            throw std::invalid_argument("Invalid barrier ID");
        usedIds.push_back(id);
        const int x1 = readInteger(wall, QStringLiteral("x1"));
        const int y1 = readInteger(wall, QStringLiteral("y1"));
        const int x2 = readInteger(wall, QStringLiteral("x2"));
        const int y2 = readInteger(wall, QStringLiteral("y2"));
        const int thickness = readInteger(wall, QStringLiteral("thicknessCm"));
        const WallMaterial material = materialFromName(wall.value(QStringLiteral("material")).toString());
        candidate.validateBarrier(x1, y1, x2, y2, thickness);
        candidate.m_barriers.push_back({id, x1, y1, x2, y2, thickness, material});
        largestId = std::max(largestId, id);
    }
    candidate.m_nextId = largestId + 1;
    candidate.recalculate();
    *this = std::move(candidate);
}

QImage MapModel::heatmapImage() const
{
    QImage image(m_length, m_width, QImage::Format_RGB32);
    for (int y = 0; y < m_width; ++y) {
        for (int x = 0; x < m_length; ++x) {
            const QColor color = parcelAt(x, y).color;
            image.setPixelColor(x, m_width - 1 - y, qRgb(color.red(), color.green(), color.blue()));
        }
    }
    return image;
}

QImage MapModel::renderImage(int cellSize, bool showGrid, int gridSize) const
{
    cellSize = std::clamp(cellSize, 1, 32);
    gridSize = std::clamp(gridSize, 1, 50);
    QImage image(m_length * cellSize, m_width * cellSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.drawImage(QRect(0, 0, image.width(), image.height()), heatmapImage());
    for (const Barrier &barrier : m_barriers) {
        QColor color;
        switch (barrier.material) {
        case WallMaterial::Plaster: color = QColor(255, 204, 204); break;
        case WallMaterial::Wood: color = QColor(153, 102, 51); break;
        case WallMaterial::Brick: color = QColor(204, 102, 0); break;
        case WallMaterial::Cement: color = QColor(160, 160, 160); break;
        case WallMaterial::ReinforcedConcrete: color = QColor(100, 100, 100); break;
        case WallMaterial::Metal: color = QColor(0, 128, 192); break;
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        const int x = std::min(barrier.x1, barrier.x2);
        const int y = m_width - 1 - std::max(barrier.y1, barrier.y2);
        const int length = std::abs(barrier.x2 - barrier.x1) + 1;
        const int width = std::abs(barrier.y2 - barrier.y1) + 1;
        const int thickness = std::clamp(cellSize * barrier.thicknessCm / 60, 2, cellSize);
        painter.drawRect(QRect(x * cellSize, y * cellSize,
                               barrier.x1 == barrier.x2 ? thickness : length * cellSize,
                               barrier.y1 == barrier.y2 ? thickness : width * cellSize));
    }
    painter.setPen(Qt::black);
    for (const AccessPoint &point : m_accessPoints) {
        const QPoint center(point.x * cellSize + cellSize / 2,
                            (m_width - 1 - point.y) * cellSize + cellSize / 2);
        painter.setBrush(Qt::white);
        painter.drawEllipse(center, std::max(2, cellSize / 2), std::max(2, cellSize / 2));
        painter.setBrush(Qt::black);
        painter.drawEllipse(center, std::max(1, cellSize / 4), std::max(1, cellSize / 4));
    }
    if (showGrid && cellSize >= 2) {
        painter.setPen(QPen(QColor(0, 0, 0, 45), 1));
        for (int x = 0; x <= m_length; x += gridSize)
            painter.drawLine(x * cellSize, 0, x * cellSize, image.height());
        for (int y = 0; y <= m_width; y += gridSize)
            painter.drawLine(0, y * cellSize, image.width(), y * cellSize);
    }
    return image;
}
