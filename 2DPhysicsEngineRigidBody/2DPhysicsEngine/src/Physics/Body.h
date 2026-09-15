#ifndef BODY_H
#define BODY_H

#include "Vec2.h"
#include "shape.h"

struct Body {
    bool isColliding = false;

    // linear movement
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    Vec2 sumForces;
    float mass;
    float invMass;

    // angular movement
    float rotation;
    float angularVelocity;
    float angularAcceleration;
    float I;
    float invI;
    float sumTorque;

    // collision
    float restitution;

    Shape* shape = nullptr;

    Body(const Shape& shape, float x, float y, float mass);
    ~Body();

    bool IsStatic() const;

    void AddForce(const Vec2& force);
    void AddTorque(float torque);
    void ClearForces();
    void ClearTorque();

    void IntegrateLinear(float dt);
    void IntegrateAngular(float dt);

    void Update(float dt);
    void ApplyImpulse(const Vec2& j);
};

#endif
