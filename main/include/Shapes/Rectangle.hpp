#pragma once
#include "Shape.hpp"

struct RectangleParams : public ShapeParams {
    int width;
    int height;
    bool fill;

    RectangleParams(int x, int y, const Color &color, int width, int height,
                    bool fill = false, int z = 0)
        : ShapeParams(x, y, color, z), width(width), height(height),
          fill(fill) {}
};

class Rectangle : public Shape {
  private:
    int width;
    int height;
    bool fill;

  public:
    Rectangle(const RectangleParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;

  private:
    std::vector<std::pair<int, int>> getVertices();
    Pixels getInsidePoints(const std::vector<std::pair<int, int>> &vertices);
};
