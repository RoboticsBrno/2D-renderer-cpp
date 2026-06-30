#include "Shapes/Point.hpp"
#include <memory>

Point::Point(const ShapeParams &params) : Shape(params) {}

std::unique_ptr<Collider> Point::defaultCollider() {
    return std::make_unique<PointCollider>(x, y);
}

void Point::drawAntiAliased(Display &displayGrid) {
    addPixel(displayGrid, x, y, 255);
}

void Point::drawAliased(Display &displayGrid) {
    addPixel(displayGrid, x, y, 255);
}
