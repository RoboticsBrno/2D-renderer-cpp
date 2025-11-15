#include "Polygon.hpp"

Polygon::Polygon(const PolygonParams &params)
    : Shape(params), vertices(params.vertices), fill(params.fill) {
    // Convert relative vertices to absolute
    for (auto &vertex : vertices) {
        vertex.first += params.x;
        vertex.second += params.y;
    }
}

Collider *Polygon::defaultCollider() {
    return new PolygonCollider(x, y, vertices);
}

std::vector<std::pair<float, float>> Polygon::getTransformedVertices() {
    std::vector<std::pair<float, float>> transformed;
    for (const auto &v : vertices) {
        transformed.push_back(getTransformedPosition(v.first, v.second));
    }
    return transformed;
}

std::vector<LineSegment *> Polygon::getSegments() {
    auto vertices = getTransformedVertices();
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

Pixels Polygon::drawAliased() {
    Pixels points;
    auto segments = getSegments();

    for (LineSegment *segment : segments) {
        Pixels segmentPoints = segment->drawAliased();
        points.insert(points.end(), segmentPoints.begin(), segmentPoints.end());
        delete segment;
    }

    if (fill) {
        Pixels insidePoints =
            getInsidePointsWithTexture(getTransformedVertices());
        points.insert(points.end(), insidePoints.begin(), insidePoints.end());
    }

    return points;
}

Pixels Polygon::drawAntiAliased() {
    Pixels points;
    auto segments = getSegments();

    for (LineSegment *segment : segments) {
        Pixels segmentPoints = segment->drawAntiAliased();
        points.insert(points.end(), segmentPoints.begin(), segmentPoints.end());
        delete segment;
    }

    if (fill) {
        Pixels insidePoints =
            getInsidePointsWithTexture(getTransformedVertices());
        points.insert(points.end(), insidePoints.begin(), insidePoints.end());
    }

    return points;
}

Pixels Polygon::getInsidePointsWithTexture(
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
