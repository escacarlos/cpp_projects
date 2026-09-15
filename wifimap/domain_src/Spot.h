#ifndef SPOT_H
#define SPOT_H

namespace wifimap {

class Spot {
protected:
    int x, y;

public:
    Spot(int px, int py);
    virtual ~Spot() = default;

    int getX() const;
    int getY() const;

    virtual bool isAccessPoint() const { return false; }
};

} // namespace wifimap

#endif // SPOT_H
