#pragma once
#include "Shape.hpp"

struct LineSegmentParams : public ShapeParams {
    float x2;
    float y2;

    LineSegmentParams(float x, float y, const Color &color, float z, float x2,
                      float y2)
        : ShapeParams(x, y, color, z), x2(x2), y2(y2) {}
};

class LineSegment : public Shape {
  private:
    float x2;
    float y2;

  public:
    LineSegment(const LineSegmentParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;
};
