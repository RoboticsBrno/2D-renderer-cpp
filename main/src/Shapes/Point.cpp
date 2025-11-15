#include "Point.hpp"

Point::Point(const ShapeParams &params) : Shape(params) {}

Collider *Point::defaultCollider() { return new PointCollider(x, y); }

Pixels Point::drawAntiAliased() {
    Color sampledColor = sampleTexture(x, y);
    return {Pixel(static_cast<int>(x), static_cast<int>(y), sampledColor)};
}

Pixels Point::drawAliased() {
    Color sampledColor = sampleTexture(x, y);
    return {Pixel(static_cast<int>(x), static_cast<int>(y), sampledColor)};
}
