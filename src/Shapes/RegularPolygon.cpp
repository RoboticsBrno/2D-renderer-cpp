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

void RegularPolygon::setSides(int sides) { this->sides = sides; }

void RegularPolygon::setRadius(int radius) {
    this->radius = radius;
    this->useSideLength = false;
}

int RegularPolygon::calculateRadiusFromSideLength(int sideLength) {
    return sideLength / (2 * std::sin(M_PI / sides));
}

void RegularPolygon::getInsidePoints(
    Pixels &points, const std::vector<std::pair<int, int>> &vertices) {
    if (vertices.size() < 3)
        return;

    int minY = vertices[0].second, maxY = vertices[0].second;
    for (const auto &v : vertices) {
        if (v.second < minY)
            minY = v.second;
        if (v.second > maxY)
            maxY = v.second;
    }

    for (int y = minY; y <= maxY; y++) {
        std::vector<int> nodes;
        size_t n = vertices.size();

        for (size_t i = 0; i < n; i++) {
            size_t j = (i + 1) % n;
            int xi = vertices[i].first, yi = vertices[i].second;
            int xj = vertices[j].first, yj = vertices[j].second;

            if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                int x = xi + (int)((float)(y - yi) * (xj - xi) / (yj - yi));
                nodes.push_back(x);
            }
        }

        std::sort(nodes.begin(), nodes.end());

        for (size_t k = 0; k < nodes.size(); k += 2) {
            if (k + 1 >= nodes.size())
                break;

            int startX = nodes[k];
            int endX = nodes[k + 1];

            for (int x = startX; x <= endX; x++) {
                points.push_back(Pixel(x, y, sampleTexture(x, y)));
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
    int effectiveRadius;
    if (useSideLength) {
        effectiveRadius = calculateRadiusFromSideLength(sideLength);
    } else {
        effectiveRadius = radius;
    }

    std::vector<std::pair<int, int>> vertices;

    Matrix2D globalMat = getGlobalMatrix();

    for (int i = 0; i < sides; i++) {
        float angle = 2 * M_PI / sides * i - M_PI / 2;

        float vx = effectiveRadius * std::cos(angle);
        float vy = effectiveRadius * std::sin(angle);

        vertices.push_back(Shape::transformPoint(
            static_cast<int>(vx), static_cast<int>(vy), globalMat));
    }
    return vertices;
}

void RegularPolygon::drawAliased(Pixels &pixels) {
    auto vertices = getVertices();

    if (vertices.size() >= 3) {
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            bresenhamLine(pixels, vertices[i].first, vertices[i].second,
                          vertices[j].first, vertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(pixels, vertices);
    }
}

void RegularPolygon::drawAntiAliased(Pixels &pixels) {
    auto vertices = getVertices();

    if (vertices.size() >= 3) {
        for (size_t i = 0; i < vertices.size(); i++) {
            size_t j = (i + 1) % vertices.size();
            wuLine(pixels, vertices[i].first, vertices[i].second,
                   vertices[j].first, vertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(pixels, vertices);
    }
}
