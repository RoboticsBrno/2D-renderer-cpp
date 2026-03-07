#include "Shapes/Rectangle.hpp"
#include "esp_timer.h"
#include <algorithm>
#include <cstdint>
#include <sys/types.h>

Rectangle::Rectangle(const RectangleParams &params)
    : Shape(params), width(params.width), height(params.height),
      fill(params.fill) {}

Collider *Rectangle::defaultCollider() {
    return new RectangleCollider(x, y, width, height);
}

std::vector<std::pair<int, int>> Rectangle::getVertices() {
    std::vector<std::pair<int, int>> vertices;

    Matrix2D globalMat = getGlobalMatrix();

    vertices.push_back(Shape::transformPoint(0, 0, globalMat));
    vertices.push_back(Shape::transformPoint(0, height - 1, globalMat));
    vertices.push_back(Shape::transformPoint(width - 1, height - 1, globalMat));
    vertices.push_back(Shape::transformPoint(width - 1, 0, globalMat));

    return vertices;
}

void Rectangle::getInsidePoints(
    Pixels &points, const std::vector<std::pair<int, int>> &vertices) {
    if (vertices.size() < 4)
        return;

    int minX = vertices[0].first, maxX = vertices[0].first;
    int minY = vertices[0].second, maxY = vertices[0].second;
    for (int i = 1; i < 4; i++) {
        minX = std::min(minX, vertices[i].first);
        maxX = std::max(maxX, vertices[i].first);
        minY = std::min(minY, vertices[i].second);
        maxY = std::max(maxY, vertices[i].second);
    }

    float u0, v0, uX, vX, uY, vY;
    getUVAt(minX, minY, u0, v0);
    getUVAt(minX + 1, minY, uX, vX);
    getUVAt(minX, minY + 1, uY, vY);

    float dux = uX - u0;
    float dvx = vX - v0;
    float duy = uY - u0;
    float dvy = vY - v0;

    int v0x = vertices[0].first, v0y = vertices[0].second;
    int dx1 = vertices[1].first - v0x, dy1 = vertices[1].second - v0y;
    int dx2 = vertices[3].first - v0x, dy2 = vertices[3].second - v0y;
    int lenSq1 = dx1 * dx1 + dy1 * dy1;
    int lenSq2 = dx2 * dx2 + dy2 * dy2;

    for (int y = minY; y <= maxY; ++y) {
        float rowU = u0 + (y - minY) * duy;
        float rowV = v0 + (y - minY) * dvy;

        for (int x = minX; x <= maxX; ++x) {
            int relX = x - v0x, relY = y - v0y;
            int dot1 = relX * dx1 + relY * dy1;
            int dot2 = relX * dx2 + relY * dy2;

            if (dot1 >= 0 && dot1 <= lenSq1 && dot2 >= 0 && dot2 <= lenSq2) {
                Color c = texture ? texture->sample((int)(rowU + 0.5f),
                                                    (int)(rowV + 0.5f))
                                  : color;
                c.a *= color.a;
                points.push_back(Pixel(x, y, c));
            }
            rowU += dux;
            rowV += dvx;
        }
    }
}

void Rectangle::drawAntiAliased(Pixels &pixels) {
    auto vertices = getVertices();

    if (vertices.size() >= 4) {
        auto tl = vertices[0];
        auto bl = vertices[1];
        auto br = vertices[2];
        auto tr = vertices[3];

        wuLine(pixels, tl.first, tl.second, tr.first, tr.second);
        wuLine(pixels, bl.first, bl.second, br.first, br.second);
        wuLine(pixels, tl.first, tl.second, bl.first, bl.second);
        wuLine(pixels, tr.first, tr.second, br.first, br.second);
    }

    if (fill) {
        getInsidePoints(pixels, vertices);
    }
}

void Rectangle::drawAliased(Pixels &pixels) {
    auto vertices = getVertices();

    if (vertices.size() >= 4) {
        auto tl = vertices[0];
        auto bl = vertices[1];
        auto br = vertices[2];
        auto tr = vertices[3];

        bresenhamLine(pixels, tl.first, tl.second, tr.first, tr.second);
        bresenhamLine(pixels, bl.first, bl.second, br.first, br.second);
        bresenhamLine(pixels, tl.first, tl.second, bl.first, bl.second);
        bresenhamLine(pixels, tr.first, tr.second, br.first, br.second);
    }

    if (fill) {
        getInsidePoints(pixels, vertices);
    }
}
