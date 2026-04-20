#include "Shapes/Point.hpp"

Point::Point(const ShapeParams &params) : Shape(params) {}

Collider *Point::defaultCollider() { return new PointCollider(x, y); }

void Point::drawAntiAliased(Display &displayGrid) {
    Color sampledColor = sampleTexture(x, y);
    setPixelSafe(displayGrid, x, y, sampledColor);
}

void Point::drawAliased(Display &displayGrid) {
    Color sampledColor = sampleTexture(x, y);
    setPixelSafe(displayGrid, x, y, sampledColor);
}
