#pragma once
#include "Shape.hpp"

struct CircleParams : public ShapeParams {
    float radius;
    bool fill;

    CircleParams(float x, float y, const Color &color, float radius,
                 bool fill = false)
        : ShapeParams(x, y, color, 0), radius(radius), fill(fill) {}
};

class Circle : public Shape {
  private:
    float radius;
    bool fill;

  public:
    Circle(const CircleParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;

  private:
    std::vector<std::pair<float, float>> getPointsToDraw(float cx, float cy,
                                                         float x, float y);
    void drawCirclePoints(Pixels &points, float cx, float cy, float x, float y);
    void drawAntiAliasedPoint(Pixels &points, float cx, float cy, float x,
                              float y, float intensity);
    void fillCircle(Pixels &points, float cx, float cy, float r);
    void fillCircleAntiAliased(Pixels &points, float cx, float cy, float r);
};
