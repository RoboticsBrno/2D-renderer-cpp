#include "LineSegment.hpp"

LineSegment::LineSegment(const LineSegmentParams &params)
    : Shape(params), x2(params.x2), y2(params.y2) {}

Collider *LineSegment::defaultCollider() {
    return new LineSegmentCollider(x, y, x2, y2);
}

Pixels LineSegment::drawAliased() {
    auto transformedStart = getTransformedPosition(x, y);
    auto transformedEnd = getTransformedPosition(x2, y2);
    float x0 = transformedStart.first;
    float y0 = transformedStart.second;
    float x1 = transformedEnd.first;
    float y1 = transformedEnd.second;

    return bresenhamLine(x0, y0, x1, y1);
}

Pixels LineSegment::drawAntiAliased() {
    auto transformedStart = getTransformedPosition(x, y);
    auto transformedEnd = getTransformedPosition(x2, y2);
    float x0 = transformedStart.first;
    float y0 = transformedStart.second;
    float x1 = transformedEnd.first;
    float y1 = transformedEnd.second;

    return wuLine(x0, y0, x1, y1);
}
