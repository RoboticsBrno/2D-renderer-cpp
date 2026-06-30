#pragma once
#include "Shape.hpp"

class Point : public Shape {
  public:
    Point(const ShapeParams &params);
    std::unique_ptr<Collider> defaultCollider() override;
    void drawAntiAliased(Display &displayGrid) override;
    void drawAliased(Display &displayGrid) override;
};
