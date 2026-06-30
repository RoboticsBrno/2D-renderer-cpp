#include "Shapes/Polygon.hpp"
#include "DrawUtils.hpp"
#include <memory>
#include <numbers>

Polygon::Polygon(const PolygonParams &params)
    : Shape(params), vertices(params.vertices), fill(params.fill) {}

std::unique_ptr<Collider> Polygon::defaultCollider() {
    return std::make_unique<PolygonCollider>(x, y, vertices);
}

std::vector<std::pair<int, int>> Polygon::getTransformedVertices() {
    std::vector<std::pair<int, int>> transformed;
    transformed.reserve(vertices.size());

    Matrix2D globalMat = getGlobalMatrix();

    for (const auto &v : vertices) {
        int x, y;
        transformPoint(v.first, v.second, globalMat, x, y);
        transformed.push_back({x, y});
    }
    return transformed;
}

void Polygon::drawAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto transformedVertices = getTransformedVertices();

    if (transformedVertices.size() >= 3) {
        for (size_t i = 0; i < transformedVertices.size(); i++) {
            size_t j = (i + 1) % transformedVertices.size();
            bresenhamLine(displayGrid, transformedVertices[i].first,
                          transformedVertices[i].second,
                          transformedVertices[j].first,
                          transformedVertices[j].second, ctx);
        }
    }

    if (fill)
        scanlineFill(displayGrid, transformedVertices, ctx);
}

void Polygon::drawAntiAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto transformedVertices = getTransformedVertices();

    if (transformedVertices.size() >= 3) {
        for (size_t i = 0; i < transformedVertices.size(); i++) {
            size_t j = (i + 1) % transformedVertices.size();
            wuLine(displayGrid, transformedVertices[i].first,
                   transformedVertices[i].second, transformedVertices[j].first,
                   transformedVertices[j].second, ctx);
        }
    }

    if (fill)
        scanlineFill(displayGrid, transformedVertices, ctx);
}
