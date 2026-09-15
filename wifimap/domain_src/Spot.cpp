#include "Spot.h"

namespace wifimap {

Spot::Spot(int px, int py) : x(px), y(py) {}

int Spot::getX() const { return x; }
int Spot::getY() const { return y; }

} // namespace wifimap