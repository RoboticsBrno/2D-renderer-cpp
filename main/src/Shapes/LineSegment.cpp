#include "LineSegment.hpp"
#include "esp_timer.h"
#include <cstdint>

LineSegment::LineSegment(const LineSegmentParams &params)
    : Shape(params), x2(params.x2), y2(params.y2) {}

Collider *LineSegment::defaultCollider() {
    return new LineSegmentCollider(x, y, x2, y2);
}

Pixels LineSegment::drawAliased() {
    auto transformedStart = getTransformedPosition(x, y);
    auto transformedEnd = getTransformedPosition(x2, y2);
    int x0 = transformedStart.first;
    int y0 = transformedStart.second;
    int x1 = transformedEnd.first;
    int y1 = transformedEnd.second;

    return bresenhamLine(x0, y0, x1, y1);
}

Pixels LineSegment::drawAntiAliased() {
    auto transformedStart = getTransformedPosition(x, y);
    auto transformedEnd = getTransformedPosition(x2, y2);
    int x0 = transformedStart.first;
    int y0 = transformedStart.second;
    int x1 = transformedEnd.first;
    int y1 = transformedEnd.second;

    Pixels points;
    points = wuLine(x0, y0, x1, y1);

    return points;
}
