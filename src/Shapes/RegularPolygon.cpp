#include "Shapes/RegularPolygon.hpp"
#include <algorithm>
#include <cmath>

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

void RegularPolygon::getInsidePoints(
    Display &displayGrid, const std::vector<std::pair<int, int>> &vertices) {
    if (vertices.size() < 3)
        return;

    int minY = vertices[0].second, maxY = vertices[0].second;
    for (const auto &v : vertices) {
        minY = std::min(minY, v.second);
        maxY = std::max(maxY, v.second);
    }

    minY = std::max(0, minY);
    maxY = std::min(displayGrid.height - 1, maxY);

    uint8_t finalAlpha = color.a;
    if (finalAlpha == 0)
        return;
    uint32_t invAlpha = 255 - finalAlpha;
    uint32_t r = color.r;
    uint32_t g = color.g;
    uint32_t b = color.b;
    bool hasTexture = (texture != nullptr);

    std::vector<int> nodes;
    nodes.reserve(16);

    size_t n = vertices.size();

    for (int y = minY; y <= maxY; y++) {
        nodes.clear();

        for (size_t i = 0; i < n; i++) {
            size_t j = (i + 1) % n;
            int xi = vertices[i].first, yi = vertices[i].second;
            int xj = vertices[j].first, yj = vertices[j].second;

            if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                nodes.push_back(xi + (y - yi) * (xj - xi) / (yj - yi));
            }
        }

        std::sort(nodes.begin(), nodes.end());

        for (size_t k = 0; k + 1 < nodes.size(); k += 2) {
            int startX = std::max(0, nodes[k]);
            int endX = std::min(displayGrid.width - 1, nodes[k + 1]);

            if (startX > endX)
                continue;

            if (!hasTexture) {
                int index = y * displayGrid.width + startX;
                Color *targetPixel = &displayGrid.pixels[index];

                for (int x = startX; x <= endX; x++) {
                    targetPixel->r =
                        (r * finalAlpha + targetPixel->r * invAlpha) >> 8;
                    targetPixel->g =
                        (g * finalAlpha + targetPixel->g * invAlpha) >> 8;
                    targetPixel->b =
                        (b * finalAlpha + targetPixel->b * invAlpha) >> 8;
                    targetPixel->a = 255;
                    targetPixel++;
                }
            } else {
                for (int x = startX; x <= endX; x++) {
                    addPixel(displayGrid, x, y, 1.0f);
                }
            }
        }
    }
}

Collider *RegularPolygon::defaultCollider() {
    int effectiveRadius =
        useSideLength ? calculateRadiusFromSideLength(sideLength) : radius;
    return new RegularPolygonCollider(x, y, sides, effectiveRadius);
}

std::vector<std::pair<int, int>> RegularPolygon::getVertices() {
    updateLocalVertices();

    std::vector<std::pair<int, int>> transformedVertices;
    transformedVertices.reserve(sides);

    Matrix2D globalMat = getGlobalMatrix();

    for (const auto &localV : localVertices) {
        int x, y;
        Shape::transformPoint(static_cast<int>(localV.first),
                              static_cast<int>(localV.second), globalMat, x, y);
        transformedVertices.push_back({x, y});
    }
    return transformedVertices;
}
void RegularPolygon::drawAliased(Display &displayGrid) {
    auto vertices = getVertices();

    if (vertices.size() >= 3) {
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            bresenhamLine(displayGrid, vertices[i].first, vertices[i].second,
                          vertices[j].first, vertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(displayGrid, vertices);
    }
}

void RegularPolygon::drawAntiAliased(Display &displayGrid) {
    auto vertices = getVertices();

    if (vertices.size() >= 3) {
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            wuLine(displayGrid, vertices[i].first, vertices[i].second,
                   vertices[j].first, vertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(displayGrid, vertices);
    }
}
