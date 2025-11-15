#include "RegularPolygon.hpp"
#include <cmath>

RegularPolygon::RegularPolygon(const RegularPolygonSideParams &params)
    : Shape(params), sides(params.sides), useSideLength(true),
      sideLength(params.sideLength), radius(0), fill(params.fill) {}

RegularPolygon::RegularPolygon(const RegularPolygonRadiusParams &params)
    : Shape(params), sides(params.sides), useSideLength(false), sideLength(0),
      radius(params.radius), fill(params.fill) {}

float RegularPolygon::calculateRadiusFromSideLength(float sideLength) {
    return sideLength / (2 * std::sin(M_PI / sides));
}

Collider *RegularPolygon::defaultCollider() {
    float effectiveRadius =
        useSideLength ? calculateRadiusFromSideLength(sideLength) : radius;
    return new RegularPolygonCollider(x, y, sides, effectiveRadius);
}

std::vector<std::pair<float, float>> RegularPolygon::getVertices() {
    float effectiveRadius;
    if (useSideLength) {
        effectiveRadius = calculateRadiusFromSideLength(sideLength);
    } else {
        effectiveRadius = radius;
    }

    std::vector<std::pair<float, float>> vertices;
    for (int i = 0; i < sides; i++) {
        float angle = 2 * M_PI / sides * i - M_PI / 2;
        float vx = x + effectiveRadius * std::cos(angle);
        float vy = y + effectiveRadius * std::sin(angle);
        auto rotated = getTransformedPosition(vx, vy);
        vertices.push_back(rotated);
    }
    return vertices;
}

std::vector<LineSegment *> RegularPolygon::getSegments() {
    auto vertices = getVertices();
    std::vector<LineSegment *> segments;

    for (size_t i = 0; i < vertices.size(); i++) {
        size_t j = (i + 1) % vertices.size();
        LineSegment *segment = new LineSegment(
            LineSegmentParams{vertices[i].first, vertices[i].second, color, z,
                              vertices[j].first, vertices[j].second});

        if (texture)
            segment->setTexture(texture);
        segment->setTextureScale(uvTransform.scaleX, uvTransform.scaleY);
        segment->setTextureOffset(uvTransform.offsetX, uvTransform.offsetY);
        segment->setFixTexture(fixTexture);

        segments.push_back(segment);
    }

    return segments;
}

Pixels RegularPolygon::drawAliased() {
    Pixels points;
    auto segments = getSegments();

    for (LineSegment *segment : segments) {
        Pixels segmentPoints = segment->drawAliased();
        points.insert(points.end(), segmentPoints.begin(), segmentPoints.end());
        delete segment;
    }

    if (fill) {
        Pixels insidePoints = getInsidePointsWithTexture(getVertices());
        points.insert(points.end(), insidePoints.begin(), insidePoints.end());
    }

    return points;
}

Pixels RegularPolygon::drawAntiAliased() {
    Pixels points;
    auto segments = getSegments();

    for (LineSegment *segment : segments) {
        Pixels segmentPoints = segment->drawAntiAliased();
        points.insert(points.end(), segmentPoints.begin(), segmentPoints.end());
        delete segment;
    }

    if (fill) {
        Pixels insidePoints = getInsidePointsWithTexture(getVertices());
        points.insert(points.end(), insidePoints.begin(), insidePoints.end());
    }

    return points;
}

Pixels RegularPolygon::getInsidePointsWithTexture(
    const std::vector<std::pair<float, float>> &vertices) {
    if (vertices.empty())
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
