#include "Shapes/Polygon.hpp"
#include <numbers>

Polygon::Polygon(const PolygonParams &params)
    : Shape(params), vertices(params.vertices), fill(params.fill) {}

Collider *Polygon::defaultCollider() {
    return new PolygonCollider(x, y, vertices);
}

std::vector<std::pair<int, int>> Polygon::getTransformedVertices() {
    std::vector<std::pair<int, int>> transformed;

    Matrix2D globalMat = getGlobalMatrix();

    for (const auto &v : vertices) {
        transformed.push_back(
            Shape::transformPoint(v.first, v.second, globalMat));
    }
    return transformed;
}

void Polygon::getInsidePoints(
    Pixels &points, const std::vector<std::pair<int, int>> &vertices) {
    if (vertices.size() < 3)
        return;

    int minY = vertices[0].second, maxY = vertices[0].second;
    for (const auto &v : vertices) {
        minY = std::min(minY, v.second);
        maxY = std::max(maxY, v.second);
    }

    for (int y = minY; y <= maxY; y++) {
        std::vector<int> nodes;
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            if ((vertices[i].second < y && vertices[j].second >= y) ||
                (vertices[j].second < y && vertices[i].second >= y)) {
                nodes.push_back(vertices[i].first +
                                (y - vertices[i].second) *
                                    (vertices[j].first - vertices[i].first) /
                                    (vertices[j].second - vertices[i].second));
            }
        }
        std::sort(nodes.begin(), nodes.end());

        for (size_t i = 0; i < nodes.size(); i += 2) {
            if (i + 1 >= nodes.size())
                break;
            for (int x = nodes[i]; x <= nodes[i + 1]; x++) {
                points.push_back(Pixel(x, y, sampleTexture(x, y)));
            }
        }
    }
}

void Polygon::drawAliased(Pixels &pixels) {
    auto transformedVertices = getTransformedVertices();

    if (transformedVertices.size() >= 3) {
        for (size_t i = 0; i < transformedVertices.size(); i++) {
            size_t j = (i + 1) % transformedVertices.size();
            bresenhamLine(pixels, transformedVertices[i].first,
                          transformedVertices[i].second,
                          transformedVertices[j].first,
                          transformedVertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(pixels, transformedVertices);
    }
}

void Polygon::drawAntiAliased(Pixels &pixels) {
    auto transformedVertices = getTransformedVertices();

    if (transformedVertices.size() >= 3) {
        for (size_t i = 0; i < transformedVertices.size(); i++) {
            size_t j = (i + 1) % transformedVertices.size();
            wuLine(pixels, transformedVertices[i].first,
                   transformedVertices[i].second, transformedVertices[j].first,
                   transformedVertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(pixels, transformedVertices);
    }
}
