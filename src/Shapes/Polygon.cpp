#include "Shapes/Polygon.hpp"
#include <numbers>

Polygon::Polygon(const PolygonParams &params)
    : Shape(params), vertices(params.vertices), fill(params.fill) {}

Collider *Polygon::defaultCollider() {
    return new PolygonCollider(x, y, vertices);
}

std::vector<std::pair<int, int>> Polygon::getTransformedVertices() {
    std::vector<std::pair<int, int>> transformed;
    transformed.reserve(vertices.size());

    Matrix2D globalMat = getGlobalMatrix();

    for (const auto &v : vertices) {
        int x, y;
        Shape::transformPoint(v.first, v.second, globalMat, x, y);
        transformed.push_back({x, y});
    }
    return transformed;
}

void Polygon::getInsidePoints(
    Display &displayGrid, const std::vector<std::pair<int, int>> &vertices) {
    if (vertices.size() < 3)
        return;

    int minY = vertices[0].second, maxY = vertices[0].second;
    for (const auto &v : vertices) {
        minY = std::min(minY, v.second);
        maxY = std::max(maxY, v.second);
    }

    minY = std::max(0, minY);
    maxY = std::min(displayGrid.height - 1, maxY);

    uint8_t finalAlpha = color.a;
    if (finalAlpha == 0)
        return;
    uint32_t invAlpha = 255 - finalAlpha;
    uint32_t r = color.r;
    uint32_t g = color.g;
    uint32_t b = color.b;
    bool hasTexture = (texture != nullptr);

    std::vector<int> nodes;
    nodes.reserve(16);

    size_t n = vertices.size();

    for (int y = minY; y <= maxY; y++) {
        nodes.clear();

        for (size_t i = 0; i < n; i++) {
            size_t j = (i + 1) % n;
            int xi = vertices[i].first, yi = vertices[i].second;
            int xj = vertices[j].first, yj = vertices[j].second;

            if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                nodes.push_back(xi + (y - yi) * (xj - xi) / (yj - yi));
            }
        }

        std::sort(nodes.begin(), nodes.end());

        for (size_t k = 0; k + 1 < nodes.size(); k += 2) {
            int startX = std::max(0, nodes[k]);
            int endX = std::min(displayGrid.width - 1, nodes[k + 1]);

            if (startX > endX)
                continue;

            if (!hasTexture) {
                int index = y * displayGrid.width + startX;
                Color *targetPixel = &displayGrid.pixels[index];

                for (int x = startX; x <= endX; x++) {
                    targetPixel->r =
                        (r * finalAlpha + targetPixel->r * invAlpha) >> 8;
                    targetPixel->g =
                        (g * finalAlpha + targetPixel->g * invAlpha) >> 8;
                    targetPixel->b =
                        (b * finalAlpha + targetPixel->b * invAlpha) >> 8;
                    targetPixel->a = 255;
                    targetPixel++;
                }
            } else {
                for (int x = startX; x <= endX; x++) {
                    addPixel(displayGrid, x, y, 1.0f);
                }
            }
        }
    }
}
void Polygon::drawAliased(Display &displayGrid) {
    auto transformedVertices = getTransformedVertices();

    if (transformedVertices.size() >= 3) {
        for (size_t i = 0; i < transformedVertices.size(); i++) {
            size_t j = (i + 1) % transformedVertices.size();
            bresenhamLine(displayGrid, transformedVertices[i].first,
                          transformedVertices[i].second,
                          transformedVertices[j].first,
                          transformedVertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(displayGrid, transformedVertices);
    }
}

void Polygon::drawAntiAliased(Display &displayGrid) {
    auto transformedVertices = getTransformedVertices();

    if (transformedVertices.size() >= 3) {
        for (size_t i = 0; i < transformedVertices.size(); i++) {
            size_t j = (i + 1) % transformedVertices.size();
            wuLine(displayGrid, transformedVertices[i].first,
                   transformedVertices[i].second, transformedVertices[j].first,
                   transformedVertices[j].second);
        }
    }

    if (fill) {
        getInsidePoints(displayGrid, transformedVertices);
    }
}
