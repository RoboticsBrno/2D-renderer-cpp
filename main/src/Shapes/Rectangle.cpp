#include "Rectangle.hpp"

Rectangle::Rectangle(const RectangleParams &params)
    : Shape(params), width(params.width), height(params.height),
      fill(params.fill) {}

Collider *Rectangle::defaultCollider() {
    return new RectangleCollider(x, y, width, height);
}

std::vector<std::pair<float, float>> Rectangle::getVertices() {
    std::vector<std::pair<float, float>> vertices;

    vertices.push_back(getTransformedPosition(x, y));              // TL
    vertices.push_back(getTransformedPosition(x, y + height - 1)); // BL
    vertices.push_back(
        getTransformedPosition(x + width - 1, y + height - 1));   // BR
    vertices.push_back(getTransformedPosition(x + width - 1, y)); // TR

    return vertices;
}

Pixels Rectangle::getInsidePoints(
    const std::vector<std::pair<float, float>> &vertices) {
    if (vertices.size() < 4)
        return Pixels();

    float minX = vertices[0].first;
    float maxX = vertices[0].first;
    float minY = vertices[0].second;
    float maxY = vertices[0].second;

    for (const auto &v : vertices) {
        minX = std::min(minX, v.first);
        maxX = std::max(maxX, v.first);
        minY = std::min(minY, v.second);
        maxY = std::max(maxY, v.second);
    }

    Pixels points;
    for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); x++) {
        for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); y++) {
            bool inside = false;
            size_t n = vertices.size();
            for (size_t i = 0, j = n - 1; i < n; j = i++) {
                float xi = vertices[i].first, yi = vertices[i].second;
                float xj = vertices[j].first, yj = vertices[j].second;

                bool intersect = ((yi > y) != (yj > y)) &&
                                 (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
                if (intersect)
                    inside = !inside;
            }

            if (inside) {
                Color sampledColor =
                    sampleTexture(static_cast<float>(x), static_cast<float>(y));
                points.push_back(Pixel(x, y, sampledColor));
            }
        }
    }

    return points;
}

Pixels Rectangle::drawAntiAliased() {
    Pixels points;
    auto vertices = getVertices();

    if (fill) {
        Pixels insidePoints = getInsidePoints(vertices);
        points.insert(points.end(), insidePoints.begin(), insidePoints.end());
    }

    if (vertices.size() >= 4) {
        auto tl = vertices[0];
        auto bl = vertices[1];
        auto br = vertices[2];
        auto tr = vertices[3];

        Pixels topEdge = wuLine(tl.first, tl.second, tr.first, tr.second);
        Pixels bottomEdge = wuLine(bl.first, bl.second, br.first, br.second);
        Pixels leftEdge = wuLine(tl.first, tl.second, bl.first, bl.second);
        Pixels rightEdge = wuLine(tr.first, tr.second, br.first, br.second);

        points.insert(points.end(), topEdge.begin(), topEdge.end());
        points.insert(points.end(), bottomEdge.begin(), bottomEdge.end());
        points.insert(points.end(), leftEdge.begin(), leftEdge.end());
        points.insert(points.end(), rightEdge.begin(), rightEdge.end());
    }

    return points;
}

Pixels Rectangle::drawAliased() {
    Pixels points;
    auto vertices = getVertices();

    if (fill) {
        Pixels insidePoints = getInsidePoints(vertices);
        points.insert(points.end(), insidePoints.begin(), insidePoints.end());
    }

    if (vertices.size() >= 4) {
        auto tl = vertices[0];
        auto bl = vertices[1];
        auto br = vertices[2];
        auto tr = vertices[3];

        Pixels topEdge =
            bresenhamLine(tl.first, tl.second, tr.first, tr.second);
        Pixels bottomEdge =
            bresenhamLine(bl.first, bl.second, br.first, br.second);
        Pixels leftEdge =
            bresenhamLine(tl.first, tl.second, bl.first, bl.second);
        Pixels rightEdge =
            bresenhamLine(tr.first, tr.second, br.first, br.second);

        points.insert(points.end(), topEdge.begin(), topEdge.end());
        points.insert(points.end(), bottomEdge.begin(), bottomEdge.end());
        points.insert(points.end(), leftEdge.begin(), leftEdge.end());
        points.insert(points.end(), rightEdge.begin(), rightEdge.end());
    }

    return points;
}
