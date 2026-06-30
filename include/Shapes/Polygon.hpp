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
    std::unique_ptr<Collider> defaultCollider() override;
    void drawAntiAliased(Display &displayGrid) override;
    void drawAliased(Display &displayGrid) override;

  private:
    std::vector<std::pair<int, int>> getTransformedVertices();
    std::vector<LineSegment *> getSegments();
    void getInsidePoints(Display &displayGrid,
                         const std::vector<std::pair<int, int>> &vertices);
};
