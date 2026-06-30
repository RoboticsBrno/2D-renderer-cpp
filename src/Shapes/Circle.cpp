#include "Shapes/Circle.hpp"
#include "DrawUtils.hpp"
#include "Utils.hpp"
#include <cmath>
#include <memory>

Circle::Circle(const CircleParams &params)
    : Shape(params), _radius(params.radius), fill(params.fill) {}

std::unique_ptr<Collider> Circle::defaultCollider() {
    return std::make_unique<CircleCollider>(_x, _y, _radius);
}

int Circle::radius() const { return _radius; }

void Circle::drawAntiAliasedPoint(Display &displayGrid, int cx, int cy, int x,
                                  int y, float intensity, const PaintCtx &ctx) {
    addPixel(displayGrid, cx + x, cy + y, intensity, ctx);
    addPixel(displayGrid, cx - x, cy + y, intensity, ctx);
    addPixel(displayGrid, cx + x, cy - y, intensity, ctx);
    addPixel(displayGrid, cx - x, cy - y, intensity, ctx);
    addPixel(displayGrid, cx + y, cy + x, intensity, ctx);
    addPixel(displayGrid, cx - y, cy + x, intensity, ctx);
    addPixel(displayGrid, cx + y, cy - x, intensity, ctx);
    addPixel(displayGrid, cx - y, cy - x, intensity, ctx);
}

void Circle::fillCircle(Display &displayGrid, int cx, int cy, int r,
                        const PaintCtx &ctx) {
    for (int y = -r; y <= r; y++) {
        int xLen = static_cast<int>(std::sqrt(r * r - y * y));
        int screenY = cy + y;
        for (int x = cx - xLen; x <= cx + xLen; x++)
            addPixel(displayGrid, x, screenY, 1.0f, ctx);
    }
}

void Circle::drawHorizontalLine(Display &displayGrid, int x1, int x2, int y,
                                const PaintCtx &ctx) {
    for (int x = x1; x <= x2; x++)
        addPixel(displayGrid, x, y, 1.0f, ctx);
}

void Circle::drawAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto center = getTransformedPosition(0, 0);
    int r = _radius;
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        drawHorizontalLine(displayGrid, center.first - x, center.first + x,
                           center.second + y, ctx);
        drawHorizontalLine(displayGrid, center.first - x, center.first + x,
                           center.second - y, ctx);
        drawHorizontalLine(displayGrid, center.first - y, center.first + y,
                           center.second + x, ctx);
        drawHorizontalLine(displayGrid, center.first - y, center.first + y,
                           center.second - x, ctx);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

void Circle::drawAntiAliased(Display &displayGrid) {
    auto ctx = makePaintCtx();
    auto center = getTransformedPosition(0, 0);
    int r = _radius;

    float sqrt2 = std::sqrt(2.0f);
    float maxX = r / sqrt2;

    for (float xPos = 0; xPos <= maxX; xPos++) {
        float yPos = std::sqrt(r * r - xPos * xPos);

        float error = yPos - std::floor(yPos);
        float intensity = error;
        float intensity2 = 1 - error;

        float y1 = std::floor(yPos);
        float y2 = y1 + 1;

        drawAntiAliasedPoint(displayGrid, center.first, center.second, xPos, y1,
                             intensity2, ctx);
        drawAntiAliasedPoint(displayGrid, center.first, center.second, xPos, y2,
                             intensity, ctx);
        drawAntiAliasedPoint(displayGrid, center.first, center.second, y1, xPos,
                             intensity2, ctx);
        drawAntiAliasedPoint(displayGrid, center.first, center.second, y2, xPos,
                             intensity, ctx);
    }

    if (fill)
        fillCircle(displayGrid, center.first, center.second, r, ctx);
}
