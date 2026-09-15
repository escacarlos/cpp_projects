#include "shape.h"

// std
#include <iostream>

/// <Circle>
/// ////////////////////////////////////////////////////////////////////////
/// </summary>
/// <param name="radius"></param>
CircleShape::CircleShape(float radius) {
	this->radius = radius;
	std::cout << "CircleShape constructor called!" << std::endl;
}

CircleShape::~CircleShape() {
	std::cout << "CircleShape destructor called!" << std::endl;
}

Shape* CircleShape::Clone() const {
	return new CircleShape(radius);
}

ShapeType CircleShape::GetType() const {
	return CIRCLE;
}

float CircleShape::GetMomentOfInertia() const {
	// For solid circles, the moment of inertia is 1/2 * r^2
	// But this still needs to be multiplied by the rigidbody's mass
	return 0.5 * radius * radius;
}

/// <Polygon>
/// ///////////////////////////////////////////////////////////////////////
/// </summary>
/// <param name="vertices"></param>
PolygonShape::PolygonShape(const std::vector<Vec2> vertices) {
	std::cout << "PolygonShape constructor called!" << std::endl;
}

PolygonShape::~PolygonShape() {
	std::cout << "Polygon destructor called!" << std::endl;
}

ShapeType PolygonShape::GetType() const {
	return POLYGON;
}

Shape* PolygonShape::Clone() const {
	return new PolygonShape(localVertices);
}

float PolygonShape::GetMomentOfInertia() const {
	return 0.0;
}

void PolygonShape::UpdateVertices(float angle, const Vec2& position) {
	// Loop all the vertices, transforming from local to world space
	for (int i = 0; i < localVertices.size(); i++) {
		// First rotate, then translate;
		worldVertices[i] = localVertices[i].Rotate(angle);
		worldVertices[i] += position;
	}
}

/// <BoxShape>
/// //////////////////////////////////////////////////////////////////////
/// </summary>
/// <param name="width"></param>
/// <param name="height"></param>
BoxShape::BoxShape(float width, float height) {
	this->width = width;
	this->height = height;

	// Load the vertices of the box polygon
	localVertices.push_back(Vec2(-width / 2.0, -height / 2.0));
	localVertices.push_back(Vec2(+width / 2.0, -height / 2.0));
	localVertices.push_back(Vec2(+width / 2.0, +height / 2.0));
	localVertices.push_back(Vec2(-width / 2.0, +height / 2.0));

	worldVertices.push_back(Vec2(-width / 2.0, -height / 2.0));
	worldVertices.push_back(Vec2(+width / 2.0, -height / 2.0));
	worldVertices.push_back(Vec2(+width / 2.0, +height / 2.0));
	worldVertices.push_back(Vec2(-width / 2.0, +height / 2.0));
	std::cout << "BoxShape constructor called!" << std::endl;
}

BoxShape::~BoxShape() {
	std::cout << "Box destructor called!" << std::endl;
}

ShapeType BoxShape::GetType() const {
	return BOX;
}

Shape* BoxShape::Clone() const {
	return new BoxShape(width, height);
}

float BoxShape::GetMomentOfInertia() const {
	// For a rectangle, the moment of inertia is 1/12 * (w^2 + h^2)
	// But this still needs to be multiplied by the rigidbody's mass
	return 0.083333 * (width * width + height * height);
}


