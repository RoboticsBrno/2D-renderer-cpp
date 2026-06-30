#include "Shapes/Point.hpp"
#include "DrawUtils.hpp"
#include <memory>

Point::Point(const ShapeParams &params) : Shape(params) {}

std::unique_ptr<Collider> Point::defaultCollider() {
    return std::make_unique<PointCollider>(x, y);
}

void Point::drawAntiAliased(Display &displayGrid) {
    addPixel(displayGrid, x, y, 1.0f, makePaintCtx());
}

void Point::drawAliased(Display &displayGrid) {
    addPixel(displayGrid, x, y, 1.0f, makePaintCtx());
}
