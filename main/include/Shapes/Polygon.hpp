#pragma once
#include "LineSegment.hpp"
#include "Shape.hpp"
#include <vector>

struct PolygonParams : public ShapeParams {
    std::vector<std::pair<float, float>> vertices;
    bool fill;

    PolygonParams(float x, float y, const Color &color,
                  const std::vector<std::pair<float, float>> &vertices,
                  bool fill = false)
        : ShapeParams(x, y, color, 0), vertices(vertices), fill(fill) {}
};

class Polygon : public Shape {
  private:
    std::vector<std::pair<float, float>> vertices;
    bool fill;

  public:
    Polygon(const PolygonParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;

  private:
    std::vector<std::pair<float, float>> getTransformedVertices();
    std::vector<LineSegment *> getSegments();
    Pixels getInsidePointsWithTexture(
        const std::vector<std::pair<float, float>> &vertices);
};
