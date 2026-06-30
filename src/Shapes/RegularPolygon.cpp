#include "Shapes/RegularPolygon.hpp"
#include "DrawUtils.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

RegularPolygon::RegularPolygon(const RegularPolygonSideParams &params)
    : Shape(params), _sides(params.sides), useSideLength(true),
      sideLength(params.sideLength), _radius(0), fill(params.fill) {}

RegularPolygon::RegularPolygon(const RegularPolygonRadiusParams &params)
    : Shape(params), _sides(params.sides), useSideLength(false), sideLength(0),
      _radius(params.radius), fill(params.fill) {}

int RegularPolygon::sides() const { return _sides; }
int RegularPolygon::radius() {
    if (useSideLength)
        return calculateRadiusFromSideLength(sideLength);
    return _radius;
}

void RegularPolygon::updateLocalVertices() {
    if (localVerticesValid)
        return;

    int effectiveRadius =
        useSideLength ? calculateRadiusFromSideLength(sideLength) : _radius;
    localVertices.clear();
    localVertices.reserve(_sides);

    for (int i = 0; i < _sides; i++) {
        float angle = 2.0f * M_PI / _sides * i - M_PI / 2.0f;
        localVertices.push_back({effectiveRadius * std::cos(angle),
                                 effectiveRadius * std::sin(angle)});
    }
    localVerticesValid = true;
}

void RegularPolygon::setSides(int sides) {
    _sides = sides;
    localVerticesValid = false;
}

void RegularPolygon::setRadius(int radius) {
    _radius = radius;
    useSideLength = false;
    localVerticesValid = false;
}

int RegularPolygon::calculateRadiusFromSideLength(int sideLength) {
    return sideLength / (2 * std::sin(M_PI / _sides));
}

std::unique_ptr<Collider> RegularPolygon::defaultCollider() {
    int effectiveRadius =
        useSideLength ? calculateRadiusFromSideLength(sideLength) : _radius;
    return std::make_unique<RegularPolygonCollider>(_x, _y, _sides, effectiveRadius);
}

std::vector<std::pair<int, int>> RegularPolygon::getVertices() {
    updateLocalVertices();

    std::vector<std::pair<int, int>> transformedVertices;
    transformedVertices.reserve(_sides);

    Matrix2D mat = globalMatrix();

    for (const auto &localV : localVertices) {
        int x, y;
        transformPoint(static_cast<int>(localV.first),
                       static_cast<int>(localV.second), mat, x, y);
        transformedVertices.push_back({x, y});
    }
    return transformedVertices;
}

void RegularPolygon::drawAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto vertices = getVertices();

    if (vertices.size() >= 3) {
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            bresenhamLine(displayGrid, vertices[i].first, vertices[i].second,
                          vertices[j].first, vertices[j].second, ctx);
        }
    }

    if (fill)
        scanlineFill(displayGrid, vertices, ctx);
}

void RegularPolygon::drawAntiAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto vertices = getVertices();

    if (vertices.size() >= 3) {
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            wuLine(displayGrid, vertices[i].first, vertices[i].second,
                   vertices[j].first, vertices[j].second, ctx);
        }
    }

    if (fill)
        scanlineFill(displayGrid, vertices, ctx);
}
