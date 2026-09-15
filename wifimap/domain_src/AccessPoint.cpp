#include "AccessPoint.h"

namespace wifimap {

AccessPoint::AccessPoint(int px, int py, float freq, int strength)
    : Spot(px, py), frequency(freq), emissionStrength(strength) {}

float AccessPoint::getFrequency() const { return frequency; }
void AccessPoint::setFrequency(float freq) { frequency = freq; }

int AccessPoint::getEmissionStrength() const { return emissionStrength; }
void AccessPoint::setEmissionStrength(int strength) { emissionStrength = strength; }

} // namespace wifimap
