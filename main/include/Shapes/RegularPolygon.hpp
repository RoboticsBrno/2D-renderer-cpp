#pragma once
#include "LineSegment.hpp"
#include "Shape.hpp"

struct RegularPolygonSideParams : public ShapeParams {
    int sides;
    int sideLength;
    bool fill;

    RegularPolygonSideParams(int x, int y, const Color &color, int sides,
                             int sideLength, bool fill = true, int z = 0)
        : ShapeParams(x, y, color, z), sides(sides), sideLength(sideLength),
          fill(fill) {}
};

struct RegularPolygonRadiusParams : public ShapeParams {
    int sides;
    int radius;
    bool fill;

    RegularPolygonRadiusParams(int x, int y, const Color &color, int sides,
                               int radius, bool fill = true)
        : ShapeParams(x, y, color, 0), sides(sides), radius(radius),
          fill(fill) {}
};

class RegularPolygon : public Shape {
  private:
    int sides;
    bool useSideLength;
    int sideLength;
    int radius;
    bool fill;

  public:
    RegularPolygon(const RegularPolygonSideParams &params);
    RegularPolygon(const RegularPolygonRadiusParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;
    int getSides() const;
    int getRadius();

  private:
    int calculateRadiusFromSideLength(int sideLength);
    std::vector<std::pair<int, int>> getVertices();
    std::vector<LineSegment *> getSegments();
    Pixels getInsidePoints(const std::vector<std::pair<int, int>> &vertices);

    Pixels getInsidePointsWithTexture(
        const std::vector<std::pair<int, int>> &vertices);
};
