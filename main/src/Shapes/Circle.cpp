#include "Circle.hpp"
#include "../Profiler.hpp"
#include "../Utils.hpp"
#include <cmath>

Circle::Circle(const CircleParams &params)
    : Shape(params), radius(params.radius), fill(params.fill) {}

Collider *Circle::defaultCollider() { return new CircleCollider(x, y, radius); }

int Circle::getRadius() const { return radius; }

std::vector<std::pair<int, int>> Circle::getPointsToDraw(int cx, int cy, int x,
                                                         int y) {
    return {{cx + x, cy + y}, {cx - x, cy + y}, {cx + x, cy - y},
            {cx - x, cy - y}, {cx + y, cy + x}, {cx - y, cy + x},
            {cx + y, cy - x}, {cx - y, cy - x}};
}

void Circle::drawCirclePoints(Pixels &points, int cx, int cy, int x, int y) {
    auto pointsToDraw = getPointsToDraw(cx, cy, x, y);
    for (const auto &point : pointsToDraw) {
        Color sampledColor = sampleTexture(point.first, point.second);
        points.push_back(Pixel(static_cast<int>(std::round(point.first)),
                               static_cast<int>(std::round(point.second)),
                               sampledColor));
    }
}

void Circle::drawAntiAliasedPoint(Pixels &points, int cx, int cy, int x, int y,
                                  float intensity) {
    auto pointsToDraw = getPointsToDraw(cx, cy, x, y);
    for (const auto &point : pointsToDraw) {
        int clampedX = point.first;
        int clampedY = point.second;

        if (intensity > 0.01f && clampedX >= 0 && clampedY >= 0) {
            Color sampledColor = sampleTexture(point.first, point.second);
            points.push_back(Pixel(
                clampedX, clampedY,
                Color(sampledColor.r, sampledColor.g, sampledColor.b,
                      std::max(0.0f,
                               std::min(1.0f, intensity * sampledColor.a)))));
        }
    }
}

void Circle::fillCircle(Pixels &points, int cx, int cy, int r) {
    int r2 = r * r;
    int minY = cy - r;
    int maxY = cy + r;

    points.reserve(points.size() + (2 * r + 1) * (2 * r + 1));

    for (int y = minY; y <= maxY; y++) {
        int dy = y - cy;
        int dy2 = dy * dy;

        if (dy2 > r2)
            continue;

        int dx = static_cast<int>(std::sqrt(r2 - dy2));
        int startX = cx - dx;
        int endX = cx + dx;

        for (int x = startX; x <= endX; x++) {
            Color sampledColor = sampleTexture(x, y);
            sampledColor.a = 1.0f;
            points.push_back(Pixel(x, y, sampledColor));
        }
    }
}

void Circle::fillCircleAntiAliased(Pixels &points, int cx, int cy, int r) {
    int r2 = r * r;
    int minY = cy - r;
    int maxY = cy + r;
    int minX = cx - r;
    int maxX = cx + r;

    points.reserve(points.size() + (2 * r + 1) * (2 * r + 1));

    for (int y = minY; y <= maxY; y++) {
        int dy = y - cy;
        int dy2 = dy * dy;

        if (dy2 > r2)
            continue;

        for (int x = minX; x <= maxX; x++) {
            int dx = x - cx;
            int dx2 = dx * dx;

            int dist2 = dx2 + dy2;

            if (dist2 <= r2) {
                Color sampledColor = sampleTexture(x, y);
                float alpha = sampledColor.a;

                if (alpha > 0.01f) {
                    points.push_back(Pixel(x, y,
                                           Color(sampledColor.r, sampledColor.g,
                                                 sampledColor.b, alpha)));
                }
            }
        }
    }
}

void Circle::drawAliased(Pixels &pixels) {
    auto center = getTransformedPosition(x, y);
    int r = radius;

    int xPos = 0;
    int yPos = r;
    int d = 1 - r;

    while (xPos <= yPos) {
        drawCirclePoints(pixels, center.first, center.second, xPos, yPos);

        if (d < 0) {
            d = d + 2 * xPos + 3;
        } else {
            d = d + 2 * (xPos - yPos) + 5;
            yPos--;
        }
        xPos++;
    }

    if (fill) {
        fillCircle(pixels, center.first, center.second, r);
    }
}

void Circle::drawAntiAliased(Pixels &pixels) {
    PROFILE_START();
    auto center = getTransformedPosition(x, y);
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

        drawAntiAliasedPoint(pixels, center.first, center.second, xPos, y1,
                             intensity2);
        drawAntiAliasedPoint(pixels, center.first, center.second, xPos, y2,
                             intensity);
        drawAntiAliasedPoint(pixels, center.first, center.second, y1, xPos,
                             intensity2);
        drawAntiAliasedPoint(pixels, center.first, center.second, y2, xPos,
                             intensity);
    }

    if (fill) {
        fillCircleAntiAliased(pixels, center.first, center.second, r);
    }
    PROFILE_END("Circle::drawAntiAliased");
}
