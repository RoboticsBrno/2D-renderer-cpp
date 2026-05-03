#include "Shapes/Point.hpp"

Point::Point(const ShapeParams &params) : Shape(params) {}

Collider *Point::defaultCollider() { return new PointCollider(x, y); }

void Point::drawAntiAliased(Display &displayGrid) {
    addPixel(displayGrid, x, y, 255);
}

void Point::drawAliased(Display &displayGrid) {
    addPixel(displayGrid, x, y, 255);
}
