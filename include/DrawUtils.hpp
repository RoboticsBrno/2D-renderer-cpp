#pragma once
#include "Utils.hpp"
#include <array>
#include <vector>

struct Matrix2D;

struct PaintCtx {
    Color color;
    Texture *texture;
    float tex_A, tex_B, tex_C;
    float tex_D, tex_E, tex_F;
};

void transformPoint(int x, int y, const Matrix2D &m, int &outX, int &outY);
Color sampleTexture(const PaintCtx &ctx, int x, int y);
void addPixel(Display &grid, int x, int y, float alpha, const PaintCtx &ctx);
void bresenhamLine(Display &grid, int x0, int y0, int x1, int y1,
                   const PaintCtx &ctx);
void wuLine(Display &grid, int x0, int y0, int x1, int y1, const PaintCtx &ctx);
void scanlineFill(Display &grid,
                  const std::vector<std::pair<int, int>> &vertices,
                  const PaintCtx &ctx);
void scanlineFill(Display &grid,
                  const std::array<std::pair<int, int>, 4> &vertices,
                  const PaintCtx &ctx);
