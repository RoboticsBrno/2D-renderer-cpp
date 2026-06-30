#pragma once
#include "Shape.hpp"
#include <array>

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
    int _width;
    int _height;
    bool fill;

  public:
    Rectangle(const RectangleParams &params);
    std::unique_ptr<Collider> defaultCollider() override;
    void drawAntiAliased(Display &displayGrid) override;
    void drawAliased(Display &displayGrid) override;
    int width() const { return _width; }
    int height() const { return _height; }

  private:
    std::array<std::pair<int, int>, 4> getVertices();
};
