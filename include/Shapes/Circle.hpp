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
    int radius;
    bool fill;

  public:
    Circle(const CircleParams &params);
    std::unique_ptr<Collider> defaultCollider() override;
    void drawAntiAliased(Display &displayGrid) override;
    void drawAliased(Display &displayGrid) override;
    int getRadius() const;

  private:
    std::vector<std::pair<int, int>> getPointsToDraw(int cx, int cy, int x,
                                                     int y);
    // Helpers
    void drawHorizontalLine(Display &displayGrid, int x1, int x2, int y);
    void drawOctantPointsAA(Display &displayGrid, int cx, int cy, int x, int y,
                            float alpha);

    void drawCirclePoints(Display &displayGrid, int cx, int cy, int x, int y);
    void drawAntiAliasedPoint(Display &displayGrid, int cx, int cy, int x,
                              int y, float intensity);
    void fillCircle(Display &displayGrid, int cx, int cy, int r);
};
