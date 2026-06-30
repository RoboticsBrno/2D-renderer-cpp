#include "Shapes/Point.hpp"
#include "DrawUtils.hpp"
#include <memory>

Point::Point(const ShapeParams &params) : Shape(params) {}

std::unique_ptr<Collider> Point::defaultCollider() {
    return std::make_unique<PointCollider>(_x, _y);
}

void Point::drawAntiAliased(Display &displayGrid) {
    addPixel(displayGrid, _x, _y, 1.0f, makePaintCtx());
}

void Point::drawAliased(Display &displayGrid) {
    addPixel(displayGrid, _x, _y, 1.0f, makePaintCtx());
}
