#pragma once
#include "LineSegment.hpp"
#include "Shape.hpp"
#include <vector>

struct PolygonParams : public ShapeParams {
    std::vector<std::pair<int, int>> vertices;
    bool fill;

    PolygonParams(int x, int y, const Color &color,
                  const std::vector<std::pair<int, int>> &vertices,
                  bool fill = false, int z = 0)
        : ShapeParams(x, y, color, z), vertices(vertices), fill(fill) {}
};

class Polygon : public Shape {
  private:
    std::vector<std::pair<int, int>> vertices;
    bool fill;

  public:
    Polygon(const PolygonParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;

  private:
    std::vector<std::pair<int, int>> getTransformedVertices();
    std::vector<LineSegment *> getSegments();
    Pixels getInsidePoints(const std::vector<std::pair<int, int>> &vertices);
    Pixels getInsidePointsWithTexture(
        const std::vector<std::pair<int, int>> &vertices);
};
