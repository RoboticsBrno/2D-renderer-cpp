#include "Shapes/Circle.hpp"
#include "Utils.hpp"
#include <cmath>
#include <memory>

Circle::Circle(const CircleParams &params)
    : Shape(params), radius(params.radius), fill(params.fill) {}

std::unique_ptr<Collider> Circle::defaultCollider() {
    return std::make_unique<CircleCollider>(x, y, radius);
}

int Circle::getRadius() const { return radius; }

std::vector<std::pair<int, int>> Circle::getPointsToDraw(int cx, int cy, int x,
                                                         int y) {
    return {{cx + x, cy + y}, {cx - x, cy + y}, {cx + x, cy - y},
            {cx - x, cy - y}, {cx + y, cy + x}, {cx - y, cy + x},
            {cx + y, cy - x}, {cx - y, cy - x}};
}

void Circle::drawCirclePoints(Display &displayGrid, int cx, int cy, int x,
                              int y) {
    auto pointsToDraw = getPointsToDraw(cx, cy, x, y);
    for (const auto &point : pointsToDraw) {
        addPixel(displayGrid, static_cast<int>(std::round(point.first)),
                 static_cast<int>(std::round(point.second)), 1.0f);
    }
}

void Circle::drawAntiAliasedPoint(Display &displayGrid, int cx, int cy, int x,
                                  int y, float intensity) {
    addPixel(displayGrid, cx + x, cy + y, intensity);
    addPixel(displayGrid, cx - x, cy + y, intensity);
    addPixel(displayGrid, cx + x, cy - y, intensity);
    addPixel(displayGrid, cx - x, cy - y, intensity);
    addPixel(displayGrid, cx + y, cy + x, intensity);
    addPixel(displayGrid, cx - y, cy + x, intensity);
    addPixel(displayGrid, cx + y, cy - x, intensity);
    addPixel(displayGrid, cx - y, cy - x, intensity);
}

void Circle::fillCircle(Display &displayGrid, int cx, int cy, int r) {

    for (int y = -r; y <= r; y++) {
        int xLen = static_cast<int>(std::sqrt(r * r - y * y));

        int startX = cx - xLen;
        int endX = cx + xLen;
        int screenY = cy + y;

        for (int x = startX; x <= endX; x++) {
            addPixel(displayGrid, x, screenY, 1.0f);
        }
    }
}

void Circle::drawAliased(Display &displayGrid) {
    auto center = getTransformedPosition(0, 0);
    int r = radius;
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        drawHorizontalLine(displayGrid, center.first - x, center.first + x,
                           center.second + y);
        drawHorizontalLine(displayGrid, center.first - x, center.first + x,
                           center.second - y);
        drawHorizontalLine(displayGrid, center.first - y, center.first + y,
                           center.second + x);
        drawHorizontalLine(displayGrid, center.first - y, center.first + y,
                           center.second - x);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// Helper for the inner loop
void Circle::drawHorizontalLine(Display &displayGrid, int x1, int x2, int y) {
    for (int x = x1; x <= x2; x++) {
        addPixel(displayGrid, x, y, 1.0f);
    }
}

void Circle::drawAntiAliased(Display &displayGrid) {
    auto center = getTransformedPosition(0, 0);
    int r = radius;

    // Xiaolin Wu's circle algorithm for anti-aliased rendering
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
                             intensity2);
        drawAntiAliasedPoint(displayGrid, center.first, center.second, xPos, y2,
                             intensity);
        drawAntiAliasedPoint(displayGrid, center.first, center.second, y1, xPos,
                             intensity2);
        drawAntiAliasedPoint(displayGrid, center.first, center.second, y2, xPos,
                             intensity);
    }

    if (fill) {
        fillCircle(displayGrid, center.first, center.second, r);
    }
}
