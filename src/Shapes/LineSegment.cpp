#include "Shapes/LineSegment.hpp"
#include "DrawUtils.hpp"
#include <memory>

LineSegment::LineSegment(const LineSegmentParams &params)
    : Shape(params), x2(params.x2), y2(params.y2) {}

std::unique_ptr<Collider> LineSegment::defaultCollider() {
    return std::make_unique<LineSegmentCollider>(_x, _y, x2, y2);
}

void LineSegment::drawAliased(Display &displayGrid) {
    Matrix2D mat = globalMatrix();

    int x0, y0, x1, y1;
    transformPoint(0, 0, mat, x0, y0);
    transformPoint(x2 - _x, y2 - _y, mat, x1, y1);

    bresenhamLine(displayGrid, x0, y0, x1, y1, makePaintCtx());
}

void LineSegment::drawAntiAliased(Display &displayGrid) {
    Matrix2D mat = globalMatrix();

    int x0, y0, x1, y1;
    transformPoint(0, 0, mat, x0, y0);
    transformPoint(x2 - _x, y2 - _y, mat, x1, y1);

    wuLine(displayGrid, x0, y0, x1, y1, makePaintCtx());
}
