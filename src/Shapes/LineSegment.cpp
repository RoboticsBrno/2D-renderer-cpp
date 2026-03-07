#include "Shapes/LineSegment.hpp"

LineSegment::LineSegment(const LineSegmentParams &params)
    : Shape(params), x2(params.x2), y2(params.y2) {}

Collider *LineSegment::defaultCollider() {
    return new LineSegmentCollider(x, y, x2, y2);
}

void LineSegment::drawAliased(Pixels &pixels) {
    Matrix2D globalMat = getGlobalMatrix();

    auto transformedStart = Shape::transformPoint(0, 0, globalMat);
    auto transformedEnd = Shape::transformPoint(x2 - x, y2 - y, globalMat);

    int x0 = transformedStart.first;
    int y0 = transformedStart.second;
    int x1 = transformedEnd.first;
    int y1 = transformedEnd.second;

    bresenhamLine(pixels, x0, y0, x1, y1);
}
void LineSegment::drawAntiAliased(Pixels &pixels) {
    Matrix2D globalMat = getGlobalMatrix();

    auto transformedStart = Shape::transformPoint(0, 0, globalMat);
    auto transformedEnd = Shape::transformPoint(x2 - x, y2 - y, globalMat);
    int x0 = transformedStart.first;
    int y0 = transformedStart.second;
    int x1 = transformedEnd.first;
    int y1 = transformedEnd.second;

    wuLine(pixels, x0, y0, x1, y1);
}
