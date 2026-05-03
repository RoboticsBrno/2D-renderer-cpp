#include "Shapes/Rectangle.hpp"
#include "esp_timer.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <sys/types.h>

Rectangle::Rectangle(const RectangleParams &params)
    : Shape(params), width(params.width), height(params.height),
      fill(params.fill) {}

Collider *Rectangle::defaultCollider() {
    return new RectangleCollider(x, y, width, height);
}

void Rectangle::getInsidePoints(
    Display &displayGrid, const std::array<std::pair<int, int>, 4> &vertices) {

    int minY = std::min({vertices[0].second, vertices[1].second,
                         vertices[2].second, vertices[3].second});
    int maxY = std::max({vertices[0].second, vertices[1].second,
                         vertices[2].second, vertices[3].second});

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

    if (hasTexture) {
        if (fixTexture && rotation.angle != 0)
            updateTrigCache();
        if (uvTransform.rotation != 0)
            updateTextureTrigCache();

        updateTextureMatrix();
    }

    struct Edge {
        int yMin, yMax;
        int32_t x_fp;
        int32_t dx_fp;
    };

    Edge edges[4];
    int numEdges = 0;

    auto processEdge = [&](const std::pair<int, int> &p1,
                           const std::pair<int, int> &p2) {
        int x1 = p1.first, y1 = p1.second;
        int x2 = p2.first, y2 = p2.second;

        if (y1 == y2)
            return;

        if (y1 > y2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        edges[numEdges].yMin = y1;
        edges[numEdges].yMax = y2;
        edges[numEdges].x_fp = x1 << 16;
        edges[numEdges].dx_fp = static_cast<int32_t>(
            (static_cast<int64_t>(x2 - x1) << 16) / (y2 - y1));
        numEdges++;
    };

    processEdge(vertices[0], vertices[1]);
    processEdge(vertices[1], vertices[2]);
    processEdge(vertices[2], vertices[3]);
    processEdge(vertices[3], vertices[0]);

    for (int y = minY; y <= maxY; ++y) {
        int intersections[2];
        int count = 0;

        for (int i = 0; i < numEdges; ++i) {
            if (edges[i].yMin < y && edges[i].yMax >= y) {
                intersections[count++] =
                    (edges[i].x_fp + (y - edges[i].yMin) * edges[i].dx_fp) >>
                    16;
                if (count == 2)
                    break;
            }
        }

        if (count < 2)
            continue;

        int xStart = intersections[0];
        int xEnd = intersections[1];
        if (xStart > xEnd)
            std::swap(xStart, xEnd);

        xStart = std::max(0, xStart);
        xEnd = std::min(displayGrid.width - 1, xEnd);

        if (xStart > xEnd)
            continue;

        int index = y * displayGrid.width + xStart;
        Color *targetPixel = &displayGrid.pixels[index];

        if (!hasTexture) {
            for (int x = xStart; x <= xEnd; ++x) {
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
            float cur_u = tex_A * xStart + tex_B * y + tex_C;
            float cur_v = tex_D * xStart + tex_E * y + tex_F;

            for (int x = xStart; x <= xEnd; ++x) {
                int u = static_cast<int>(cur_u + 0.5f);
                int v = static_cast<int>(cur_v + 0.5f);

                Color texColor = texture->sample(u, v);

                uint32_t sampledAlpha = (texColor.a * color.a) >> 8;
                uint32_t texAlpha = (finalAlpha * sampledAlpha) >> 8;
                uint32_t invTexAlpha = 255 - texAlpha;

                targetPixel->r =
                    (texColor.r * texAlpha + targetPixel->r * invTexAlpha) >> 8;
                targetPixel->g =
                    (texColor.g * texAlpha + targetPixel->g * invTexAlpha) >> 8;
                targetPixel->b =
                    (texColor.b * texAlpha + targetPixel->b * invTexAlpha) >> 8;
                targetPixel->a = 255;
                targetPixel++;

                cur_u += tex_A;
                cur_v += tex_D;
            }
        }
    }
}

std::array<std::pair<int, int>, 4> Rectangle::getVertices() {
    Matrix2D globalMat = getGlobalMatrix();

    int x0, y0, x1, y1, x2, y2, x3, y3;

    Shape::transformPoint(0, 0, globalMat, x0, y0);
    Shape::transformPoint(0, height - 1, globalMat, x1, y1);
    Shape::transformPoint(width - 1, height - 1, globalMat, x2, y2);
    Shape::transformPoint(width - 1, 0, globalMat, x3, y3);

    return {std::pair<int, int>{x0, y0}, std::pair<int, int>{x1, y1},
            std::pair<int, int>{x2, y2}, std::pair<int, int>{x3, y3}};
}

void Rectangle::drawAntiAliased(Display &displayGrid) {
    auto vertices = getVertices();

    auto tl = vertices[0];
    auto bl = vertices[1];
    auto br = vertices[2];
    auto tr = vertices[3];

    wuLine(displayGrid, tl.first, tl.second, tr.first, tr.second);
    wuLine(displayGrid, bl.first, bl.second, br.first, br.second);
    wuLine(displayGrid, tl.first, tl.second, bl.first, bl.second);
    wuLine(displayGrid, tr.first, tr.second, br.first, br.second);

    if (fill) {
        getInsidePoints(displayGrid, vertices);
    }
}

void Rectangle::drawAliased(Display &displayGrid) {
    auto vertices = getVertices();

    auto tl = vertices[0];
    auto bl = vertices[1];
    auto br = vertices[2];
    auto tr = vertices[3];

    bresenhamLine(displayGrid, tl.first, tl.second, tr.first, tr.second);
    bresenhamLine(displayGrid, bl.first, bl.second, br.first, br.second);
    bresenhamLine(displayGrid, tl.first, tl.second, bl.first, bl.second);
    bresenhamLine(displayGrid, tr.first, tr.second, br.first, br.second);

    if (fill) {
        getInsidePoints(displayGrid, vertices);
    }
}
