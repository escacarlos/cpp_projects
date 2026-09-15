#ifndef ACCESSPOINT_H
#define ACCESSPOINT_H

#include "Spot.h"

namespace wifimap {

class AccessPoint : public Spot {
private:
    float frequency;
    int emissionStrength;

public:
    AccessPoint(int px, int py, float freq, int strength);

    float getFrequency() const;
    void setFrequency(float freq);

    int getEmissionStrength() const;
    void setEmissionStrength(int strength);

    bool isAccessPoint() const override { return true; }
};

} // namespace wifimap

#endif // ACCESSPOINT_H
