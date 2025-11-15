#pragma once
#include "Shape.hpp"

struct RectangleParams : public ShapeParams {
    float width;
    float height;
    bool fill;

    RectangleParams(float x, float y, const Color &color, float width,
                    float height, bool fill = false)
        : ShapeParams(x, y, color, 0), width(width), height(height),
          fill(fill) {}
};

class Rectangle : public Shape {
  private:
    float width;
    float height;
    bool fill;

  public:
    Rectangle(const RectangleParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;

  private:
    std::vector<std::pair<float, float>> getVertices();
    Pixels
    getInsidePoints(const std::vector<std::pair<float, float>> &vertices);
};
