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

    // 1. Setup vertices and pre-compute edge vectors for the cross-product
    int vx[4], vy[4], ex[4], ey[4];
    for (int i = 0; i < 4; i++) {
        vx[i] = vertices[i].first;
        vy[i] = vertices[i].second;
        minX = std::min(minX, vx[i]);
        maxX = std::max(maxX, vx[i]);
        minY = std::min(minY, vy[i]);
        maxY = std::max(maxY, vy[i]);
    }

    for (int i = 0; i < 4; i++) {
        ex[i] = vertices[(i + 1) % 4].first - vx[i];
        ey[i] = vertices[(i + 1) % 4].second - vy[i];
    }

    float u0, v0, uX, vX, uY, vY;
    getUVAt(minX, minY, u0, v0);
    getUVAt(minX + 1, minY, uX, vX);
    getUVAt(minX, minY + 1, uY, vY);

    float dux = uX - u0;
    float dvx = vX - v0;
    float duy = uY - u0;
    float dvy = vY - v0;

    for (int y = minY; y <= maxY; ++y) {
        float rowU = u0 + (y - minY) * duy;
        float rowV = v0 + (y - minY) * dvy;

        for (int x = minX; x <= maxX; ++x) {

            bool inside = true;
            bool hasPos = false;
            bool hasNeg = false;

            for (int i = 0; i < 4; ++i) {
                int cross = (x - vx[i]) * ey[i] - (y - vy[i]) * ex[i];

                if (cross > 0)
                    hasPos = true;
                else if (cross < 0)
                    hasNeg = true;

                if (hasPos && hasNeg) {
                    inside = false;
                    break;
                }
            }

            if (inside) {
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
