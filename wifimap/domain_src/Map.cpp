#include <stdexcept>

#include "Map.h"

namespace wifimap {

int Map::getIDSpot(int x, int y) const
{
    return y * getLength() + x;
}

int Map::getCoordinateX(int id) const
{
    return id % getLength();
}

int Map::getCoordinateY(int id) const
{
    return static_cast<int>(id / getLength());
}

Map::Map(int p_length, int p_width) : length{p_length}, width{p_width}
{
    for (int x = 0; x < length; x++) {
        for (int y = 0; y < width; y++) {
            wifimap.insert({getIDSpot(x, y), std::make_unique<Spot>(x, y)});
        }
    }
}

Map::Map() : Map(500, 500)
{
}

int Map::getLength() const
{
    return length;
}

void Map::setLength(int p_length)
{
    length = p_length;
}

int Map::getWidth() const
{
    return width;
}

void Map::setWidth(int p_width)
{
    width = p_width;
}

bool Map::addAccessPoint(int x, int y, float frequency, int strength)
{
    try {
        int id = getIDSpot(x, y);
        if (id < 0 || id >= getLength() * getWidth()) {
            throw std::range_error("The ID is out of range");
        }
        auto it = wifimap.find(id);
        if (it == wifimap.end()) {
            return false;
        }
        wifimap[id] = std::make_unique<AccessPoint>(x, y, frequency, strength);
        auto accessPointCopy = std::make_unique<AccessPoint>(x, y, frequency, strength);
        list_accesspoints.push_back(std::move(accessPointCopy));
        return true;
    } catch (...) {
        return false;
    }
}

bool Map::moveAccessPoint(int x, int y)
{
    return false;
}

bool Map::modifyAccessPoint(int x, int y, float frequency, int strength)
{
    return false;
}

bool Map::deleteAccessPoint(int x, int y)
{
    int id = getIDSpot(x, y);
    auto it = wifimap.find(id);
    if (it != wifimap.end() || !it->second->isAccessPoint()) {
        return false;
    }

    wifimap[id] = std::make_unique<Spot>(x, y); // Revert back to Spot
    return true;
}

Spot* Map::getSpot(int x, int y) {
    int id = getIDSpot(x, y);
    // if (id < 0 || id > getLength() * getWidth()) throw std::range_error("The ID is out of range");
    auto it = wifimap.find(id);
    if (it != wifimap.end()) {
        return it->second.get();
    }
    return nullptr;
}

AccessPoint* Map::getAccessPoint(int x, int y)
{
    Spot* spot = getSpot(x, y);
    if (spot && spot->isAccessPoint()) {
        return static_cast<AccessPoint*>(spot);
    }
    return nullptr;
}

AccessPoint* Map::getAccessPointFromList(int x, int y)
{
    int id = getIDSpot(x, y);
    if (id < 0 || id > getLength() * getWidth()) throw std::range_error("The ID is out of range");
    for (const auto& point : list_accesspoints) {
        if (point->getX() == x && point->getY() == y) {
            return point.get();
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<AccessPoint>>& Map::getListAccessPoint()
{
    return list_accesspoints;
}

}

