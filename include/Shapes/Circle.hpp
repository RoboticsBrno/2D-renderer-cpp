#pragma once
#include "Shape.hpp"

struct CircleParams : public ShapeParams {
    int radius;
    bool fill;

    CircleParams(int x, int y, const Color &color, int radius,
                 bool fill = false, int z = 0)
        : ShapeParams(x, y, color, z), radius(radius), fill(fill) {}
};

class Circle : public Shape {
  private:
    int _radius;
    bool fill;

  public:
    Circle(const CircleParams &params);
    std::unique_ptr<Collider> defaultCollider() override;
    void drawAntiAliased(Display &displayGrid) override;
    void drawAliased(Display &displayGrid) override;
    int radius() const;

  private:
    void drawHorizontalLine(Display &displayGrid, int x1, int x2, int y,
                            const PaintCtx &ctx);
    void drawAntiAliasedPoint(Display &displayGrid, int cx, int cy, int x,
                              int y, float intensity, const PaintCtx &ctx);
    void fillCircle(Display &displayGrid, int cx, int cy, int r,
                    const PaintCtx &ctx);
};
