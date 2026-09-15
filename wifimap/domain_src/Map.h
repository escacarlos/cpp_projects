#ifndef MAP_H
#define MAP_H

#include "Spot.h"
#include "AccessPoint.h"

#include <vector>
#include <unordered_map>
#include <memory>

namespace wifimap {

class Map {
private:

    int length;
    int width;
    std::unordered_map<int, std::unique_ptr<Spot>> wifimap;
    std::vector<std::unique_ptr<AccessPoint>> list_accesspoints;
    int getIDSpot(int x, int y) const;
    int getCoordinateX(int id) const;
    int getCoordinateY(int id) const;

public:

    Map(int p_length, int p_width);
    Map();
    int getLength() const;
    void setLength(int p_length);
    int getWidth() const;
    void setWidth(int p_width);
    bool addAccessPoint(int x, int y, float frequency, int strength);
    bool moveAccessPoint(int x, int y);
    bool modifyAccessPoint(int x, int y, float frequency, int strength);
    bool deleteAccessPoint(int x, int y);
    Spot* getSpot(int x, int y);
    AccessPoint* getAccessPoint(int x, int y);
    AccessPoint* getAccessPointFromList(int x, int y);
    const std::vector<std::unique_ptr<AccessPoint>>& getListAccessPoint();
};
    
} // namespace wifimap

#endif // MAP_H