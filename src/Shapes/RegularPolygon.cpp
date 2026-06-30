#include "Shapes/RegularPolygon.hpp"
#include "DrawUtils.hpp"
#include <algorithm>
#include <cmath>
#include <memory>

RegularPolygon::RegularPolygon(const RegularPolygonSideParams &params)
    : Shape(params), sides(params.sides), useSideLength(true),
      sideLength(params.sideLength), radius(0), fill(params.fill) {}

RegularPolygon::RegularPolygon(const RegularPolygonRadiusParams &params)
    : Shape(params), sides(params.sides), useSideLength(false), sideLength(0),
      radius(params.radius), fill(params.fill) {}

int RegularPolygon::getSides() const { return sides; }
int RegularPolygon::getRadius() {
    if (useSideLength) {
        return calculateRadiusFromSideLength(sideLength);
    }
    return radius;
}

void RegularPolygon::updateLocalVertices() {
    if (localVerticesValid)
        return;

    int effectiveRadius =
        useSideLength ? calculateRadiusFromSideLength(sideLength) : radius;
    localVertices.clear();
    localVertices.reserve(sides);

    for (int i = 0; i < sides; i++) {
        float angle = 2.0f * M_PI / sides * i - M_PI / 2.0f;
        localVertices.push_back({effectiveRadius * std::cos(angle),
                                 effectiveRadius * std::sin(angle)});
    }
    localVerticesValid = true;
}

void RegularPolygon::setSides(int sides) {
    this->sides = sides;
    localVerticesValid = false;
}

void RegularPolygon::setRadius(int radius) {
    this->radius = radius;
    this->useSideLength = false;
    localVerticesValid = false;
}

int RegularPolygon::calculateRadiusFromSideLength(int sideLength) {
    return sideLength / (2 * std::sin(M_PI / sides));
}

std::unique_ptr<Collider> RegularPolygon::defaultCollider() {
    int effectiveRadius =
        useSideLength ? calculateRadiusFromSideLength(sideLength) : radius;
    return std::make_unique<RegularPolygonCollider>(x, y, sides, effectiveRadius);
}

std::vector<std::pair<int, int>> RegularPolygon::getVertices() {
    updateLocalVertices();

    std::vector<std::pair<int, int>> transformedVertices;
    transformedVertices.reserve(sides);

    Matrix2D globalMat = getGlobalMatrix();

    for (const auto &localV : localVertices) {
        int x, y;
        transformPoint(static_cast<int>(localV.first),
                       static_cast<int>(localV.second), globalMat, x, y);
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
