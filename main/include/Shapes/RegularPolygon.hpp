#pragma once
#include "LineSegment.hpp"
#include "Shape.hpp"

struct RegularPolygonSideParams : public ShapeParams {
    int sides;
    float sideLength;
    bool fill;

    RegularPolygonSideParams(float x, float y, const Color &color, int sides,
                             float sideLength, bool fill = true)
        : ShapeParams(x, y, color, 0), sides(sides), sideLength(sideLength),
          fill(fill) {}
};

struct RegularPolygonRadiusParams : public ShapeParams {
    int sides;
    float radius;
    bool fill;

    RegularPolygonRadiusParams(float x, float y, const Color &color, int sides,
                               float radius, bool fill = true)
        : ShapeParams(x, y, color, 0), sides(sides), radius(radius),
          fill(fill) {}
};

class RegularPolygon : public Shape {
  private:
    int sides;
    bool useSideLength;
    float sideLength;
    float radius;
    bool fill;

  public:
    RegularPolygon(const RegularPolygonSideParams &params);
    RegularPolygon(const RegularPolygonRadiusParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;

  private:
    float calculateRadiusFromSideLength(float sideLength);
    std::vector<std::pair<float, float>> getVertices();
    std::vector<LineSegment *> getSegments();
    Pixels getInsidePointsWithTexture(
        const std::vector<std::pair<float, float>> &vertices);
};
