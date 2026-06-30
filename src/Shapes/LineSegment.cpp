#include "Shapes/LineSegment.hpp"
#include <memory>

LineSegment::LineSegment(const LineSegmentParams &params)
    : Shape(params), x2(params.x2), y2(params.y2) {}

std::unique_ptr<Collider> LineSegment::defaultCollider() {
    return std::make_unique<LineSegmentCollider>(x, y, x2, y2);
}

void LineSegment::drawAliased(Display &displayGrid) {
    Matrix2D globalMat = getGlobalMatrix();

    int x0, y0, x1, y1;
    Shape::transformPoint(0, 0, globalMat, x0, y0);
    Shape::transformPoint(x2 - x, y2 - y, globalMat, x1, y1);

    bresenhamLine(displayGrid, x0, y0, x1, y1);
}
void LineSegment::drawAntiAliased(Display &displayGrid) {
    Matrix2D globalMat = getGlobalMatrix();

    int x0, y0, x1, y1;
    Shape::transformPoint(0, 0, globalMat, x0, y0);
    Shape::transformPoint(x2 - x, y2 - y, globalMat, x1, y1);

    wuLine(displayGrid, x0, y0, x1, y1);
}
