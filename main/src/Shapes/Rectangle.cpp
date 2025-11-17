#include "Rectangle.hpp"
#include "../Profiler.hpp"

Rectangle::Rectangle(const RectangleParams &params)
    : Shape(params), width(params.width), height(params.height),
      fill(params.fill) {}

Collider *Rectangle::defaultCollider() {
    return new RectangleCollider(x, y, width, height);
}

std::vector<std::pair<int, int>> Rectangle::getVertices() {
    std::vector<std::pair<int, int>> vertices;

    vertices.push_back(getTransformedPosition(x, y));              // TL
    vertices.push_back(getTransformedPosition(x, y + height - 1)); // BL
    vertices.push_back(
        getTransformedPosition(x + width - 1, y + height - 1));   // BR
    vertices.push_back(getTransformedPosition(x + width - 1, y)); // TR

    return vertices;
}

void Rectangle::getInsidePoints(
    Pixels &points, const std::vector<std::pair<int, int>> &vertices) {
    if (vertices.size() < 4)
        return;

    int minX = vertices[0].first;
    int maxX = vertices[0].first;
    int minY = vertices[0].second;
    int maxY = vertices[0].second;

    for (const auto &v : vertices) {
        minX = std::min(minX, v.first);
        maxX = std::max(maxX, v.first);
        minY = std::min(minY, v.second);
        maxY = std::max(maxY, v.second);
    }

    for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); x++) {
        for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); y++) {
            bool inside = false;
            size_t n = vertices.size();
            for (size_t i = 0, j = n - 1; i < n; j = i++) {
                int xi = vertices[i].first, yi = vertices[i].second;
                int xj = vertices[j].first, yj = vertices[j].second;

                bool intersect = ((yi > y) != (yj > y)) &&
                                 (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
                if (intersect)
                    inside = !inside;
            }

            if (inside) {
                Color sampledColor = sampleTexture(x, y);
                points.push_back(Pixel(x, y, sampledColor));
            }
        }
    }
}

void Rectangle::drawAntiAliased(Pixels &pixels) {
    PROFILE_START();
    auto vertices = getVertices();

    if (fill) {
        getInsidePoints(pixels, vertices);
    }

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
    PROFILE_END("Rectangle::drawAntiAliased");
}

void Rectangle::drawAliased(Pixels &pixels) {
    auto vertices = getVertices();

    if (fill) {
        getInsidePoints(pixels, vertices);
    }

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
}
