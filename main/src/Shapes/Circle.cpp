#include "Circle.hpp"
#include <cmath>

Circle::Circle(const CircleParams &params)
    : Shape(params), radius(params.radius), fill(params.fill) {}

Collider *Circle::defaultCollider() { return new CircleCollider(x, y, radius); }

std::vector<std::pair<float, float>> Circle::getPointsToDraw(float cx, float cy,
                                                             float x, float y) {
    return {{cx + x, cy + y}, {cx - x, cy + y}, {cx + x, cy - y},
            {cx - x, cy - y}, {cx + y, cy + x}, {cx - y, cy + x},
            {cx + y, cy - x}, {cx - y, cy - x}};
}

void Circle::drawCirclePoints(Pixels &points, float cx, float cy, float x,
                              float y) {
    auto pointsToDraw = getPointsToDraw(cx, cy, x, y);
    for (const auto &point : pointsToDraw) {
        Color sampledColor = sampleTexture(point.first, point.second);
        points.push_back(Pixel(static_cast<int>(std::round(point.first)),
                               static_cast<int>(std::round(point.second)),
                               sampledColor));
    }
}

void Circle::drawAntiAliasedPoint(Pixels &points, float cx, float cy, float x,
                                  float y, float intensity) {
    auto pointsToDraw = getPointsToDraw(cx, cy, x, y);
    for (const auto &point : pointsToDraw) {
        int clampedX =
            static_cast<int>(std::max(0.0f, std::round(point.first)));
        int clampedY =
            static_cast<int>(std::max(0.0f, std::round(point.second)));

        if (intensity > 0.01f) {
            Color sampledColor = sampleTexture(point.first, point.second);
            points.push_back(Pixel(
                clampedX, clampedY,
                Color(sampledColor.r, sampledColor.g, sampledColor.b,
                      std::max(0.0f,
                               std::min(1.0f, intensity * sampledColor.a)))));
        }
    }
}

void Circle::fillCircle(Pixels &points, float cx, float cy, float r) {
    for (int y = -static_cast<int>(r); y <= static_cast<int>(r); y++) {
        float xDist = std::sqrt(r * r - y * y);
        int startX = static_cast<int>(std::ceil(cx - xDist));
        int endX = static_cast<int>(std::floor(cx + xDist));

        for (int x = startX; x <= endX; x++) {
            Color sampledColor = sampleTexture(static_cast<float>(x),
                                               static_cast<float>(cy + y));
            sampledColor.a = 1.0f;
            points.push_back(Pixel(static_cast<int>(std::round(x)),
                                   static_cast<int>(std::round(cy + y)),
                                   sampledColor));
        }
    }
}

void Circle::fillCircleAntiAliased(Pixels &points, float cx, float cy,
                                   float r) {
    for (int y = static_cast<int>(std::ceil(cy - r));
         y <= static_cast<int>(std::floor(cy + r)); y++) {
        for (int x = static_cast<int>(std::ceil(cx - r));
             x <= static_cast<int>(std::floor(cx + r)); x++) {
            float dx = x - cx;
            float dy = y - cy;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance <= r) {
                Color sampledColor =
                    sampleTexture(static_cast<float>(x), static_cast<float>(y));
                float alpha = sampledColor.a;

                if (alpha > 0.01f) {
                    points.push_back(Pixel(static_cast<int>(std::round(x)),
                                           static_cast<int>(std::round(y)),
                                           Color(sampledColor.r, sampledColor.g,
                                                 sampledColor.b, alpha)));
                }
            }
        }
    }
}

Pixels Circle::drawAliased() {
    Pixels points;
    auto center = getTransformedPosition(x, y);
    float r = radius;

    float xPos = 0;
    float yPos = r;
    float d = 1 - r;

    while (xPos <= yPos) {
        drawCirclePoints(points, center.first, center.second, xPos, yPos);

        if (d < 0) {
            d = d + 2 * xPos + 3;
        } else {
            d = d + 2 * (xPos - yPos) + 5;
            yPos--;
        }
        xPos++;
    }

    if (fill) {
        fillCircle(points, center.first, center.second, r);
    }

    return points;
}

Pixels Circle::drawAntiAliased() {
    Pixels points;
    auto center = getTransformedPosition(x, y);
    float r = radius;

    // Xiaolin Wu's circle algorithm for anti-aliased rendering
    for (float xPos = 0; xPos <= r / std::sqrt(2); xPos++) {
        float yPos = std::sqrt(r * r - xPos * xPos);

        float error = yPos - std::floor(yPos);
        float intensity = error;
        float intensity2 = 1 - error;

        float y1 = std::floor(yPos);
        float y2 = y1 + 1;

        drawAntiAliasedPoint(points, center.first, center.second, xPos, y1,
                             intensity2);
        drawAntiAliasedPoint(points, center.first, center.second, xPos, y2,
                             intensity);
        drawAntiAliasedPoint(points, center.first, center.second, y1, xPos,
                             intensity2);
        drawAntiAliasedPoint(points, center.first, center.second, y2, xPos,
                             intensity);
    }

    if (fill) {
        fillCircleAntiAliased(points, center.first, center.second, r);
    }

    return points;
}
