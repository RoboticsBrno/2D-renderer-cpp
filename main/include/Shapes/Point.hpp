#pragma once
#include "Shape.hpp"

class Point : public Shape {
  public:
    Point(const ShapeParams &params);
    Collider *defaultCollider() override;
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;
};
