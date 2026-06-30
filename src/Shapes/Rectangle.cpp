#include "Shapes/Rectangle.hpp"
#include "DrawUtils.hpp"
#include "esp_timer.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <sys/types.h>

Rectangle::Rectangle(const RectangleParams &params)
    : Shape(params), _width(params.width), _height(params.height),
      fill(params.fill) {}

std::unique_ptr<Collider> Rectangle::defaultCollider() {
    return std::make_unique<RectangleCollider>(_x, _y, _width, _height);
}

std::array<std::pair<int, int>, 4> Rectangle::getVertices() {
    Matrix2D mat = globalMatrix();

    int x0, y0, x1, y1, x2, y2, x3, y3;

    transformPoint(0, 0, mat, x0, y0);
    transformPoint(0, _height - 1, mat, x1, y1);
    transformPoint(_width - 1, _height - 1, mat, x2, y2);
    transformPoint(_width - 1, 0, mat, x3, y3);

    return {std::pair<int, int>{x0, y0}, std::pair<int, int>{x1, y1},
            std::pair<int, int>{x2, y2}, std::pair<int, int>{x3, y3}};
}

void Rectangle::drawAntiAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto vertices = getVertices();

    auto tl = vertices[0];
    auto bl = vertices[1];
    auto br = vertices[2];
    auto tr = vertices[3];

    wuLine(displayGrid, tl.first, tl.second, tr.first, tr.second, ctx);
    wuLine(displayGrid, bl.first, bl.second, br.first, br.second, ctx);
    wuLine(displayGrid, tl.first, tl.second, bl.first, bl.second, ctx);
    wuLine(displayGrid, tr.first, tr.second, br.first, br.second, ctx);

    if (fill)
        scanlineFill(displayGrid, vertices, ctx);
}

void Rectangle::drawAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto vertices = getVertices();

    auto tl = vertices[0];
    auto bl = vertices[1];
    auto br = vertices[2];
    auto tr = vertices[3];

    bresenhamLine(displayGrid, tl.first, tl.second, tr.first, tr.second, ctx);
    bresenhamLine(displayGrid, bl.first, bl.second, br.first, br.second, ctx);
    bresenhamLine(displayGrid, tl.first, tl.second, bl.first, bl.second, ctx);
    bresenhamLine(displayGrid, tr.first, tr.second, br.first, br.second, ctx);

    if (fill)
        scanlineFill(displayGrid, vertices, ctx);
}
