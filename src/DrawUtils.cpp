#include "DrawUtils.hpp"
#include "Shapes/Shape.hpp"
#include "Texture.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

void transformPoint(int x, int y, const Matrix2D &m, int &outX, int &outY) {
    float tx = x * m.a + y * m.c + m.e;
    float ty = x * m.b + y * m.d + m.f;
    outX = static_cast<int>(tx + (tx >= 0.0f ? 0.5f : -0.5f));
    outY = static_cast<int>(ty + (ty >= 0.0f ? 0.5f : -0.5f));
}

Color sampleTexture(const PaintCtx &ctx, int x, int y) {
    if (!ctx.texture)
        return ctx.color;
    float u = ctx.tex_A * x + ctx.tex_B * y + ctx.tex_C;
    float v = ctx.tex_D * x + ctx.tex_E * y + ctx.tex_F;
    int texU = static_cast<int>(std::round(u));
    int texV = static_cast<int>(std::round(v));
    Color texColor = ctx.texture->sample(texU, texV);
    texColor.a = (texColor.a * ctx.color.a) >> 8;
    return texColor;
}

void addPixel(Display &displayGrid, int x, int y, float alpha,
              const PaintCtx &ctx) {
    if (alpha <= 0.0f)
        return;
    if (static_cast<unsigned>(x) >= static_cast<unsigned>(displayGrid.width) ||
        static_cast<unsigned>(y) >= static_cast<unsigned>(displayGrid.height))
        return;

    uint8_t finalAlpha = static_cast<uint8_t>(alpha * ctx.color.a);
    if (finalAlpha == 0)
        return;
    uint32_t invAlpha = 255 - finalAlpha;

    int index = y * displayGrid.width + x;
    Color *targetPixel = &displayGrid.pixels[index];

    if (!ctx.texture) {
        targetPixel->r =
            (ctx.color.r * finalAlpha + targetPixel->r * invAlpha) >> 8;
        targetPixel->g =
            (ctx.color.g * finalAlpha + targetPixel->g * invAlpha) >> 8;
        targetPixel->b =
            (ctx.color.b * finalAlpha + targetPixel->b * invAlpha) >> 8;
    } else {
        Color texColor = sampleTexture(ctx, x, y);
        uint32_t texAlpha = (finalAlpha * texColor.a) >> 8;
        uint32_t invTexAlpha = 255 - texAlpha;
        targetPixel->r =
            (texColor.r * texAlpha + targetPixel->r * invTexAlpha) >> 8;
        targetPixel->g =
            (texColor.g * texAlpha + targetPixel->g * invTexAlpha) >> 8;
        targetPixel->b =
            (texColor.b * texAlpha + targetPixel->b * invTexAlpha) >> 8;
    }
    targetPixel->a = 255;
}

void bresenhamLine(Display &displayGrid, int x0, int y0, int x1, int y1,
                   const PaintCtx &ctx) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        addPixel(displayGrid, x0, y0, 1.0f, ctx);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void wuLine(Display &points, int x0_int, int y0_int, int x1_int, int y1_int,
            const PaintCtx &ctx) {
    int x0 = x0_int;
    int y0 = y0_int;
    int x1 = x1_int;
    int y1 = y1_int;

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    if (dx == 0) {
        if (x0_int >= 0 && x0_int < points.width && y0_int >= 0 &&
            y0_int < points.height) {
            points.pixels[y0_int * points.width + x0_int] = ctx.color;
        }
        return;
    }

    int32_t gradient_fp =
        static_cast<int32_t>((static_cast<int64_t>(dy) << 16) / dx);
    int32_t intery_fp = y0 << 16;

    int width = points.width;
    int height = points.height;

    if (steep) {
        for (int x = x0; x <= x1; x++) {
            if (x < 0 || x >= height) {
                intery_fp += gradient_fp;
                continue;
            }

            int y_base = intery_fp >> 16;
            uint8_t fraction = (intery_fp & 0xFFFF) >> 8;
            uint8_t inv_fraction = 255 - fraction;

            int rowIndex = x * width;

            if (y_base >= 0 && y_base < width) {
                Color src1 = sampleTexture(ctx, y_base, x);
                Color *p1 = &points.pixels[rowIndex + y_base];
                p1->r = (src1.r * inv_fraction + p1->r * fraction) >> 8;
                p1->g = (src1.g * inv_fraction + p1->g * fraction) >> 8;
                p1->b = (src1.b * inv_fraction + p1->b * fraction) >> 8;
                p1->a = 255;
            }

            if (y_base + 1 >= 0 && y_base + 1 < width) {
                Color src2 = sampleTexture(ctx, y_base + 1, x);
                Color *p2 = &points.pixels[rowIndex + y_base + 1];
                p2->r = (src2.r * fraction + p2->r * inv_fraction) >> 8;
                p2->g = (src2.g * fraction + p2->g * inv_fraction) >> 8;
                p2->b = (src2.b * fraction + p2->b * inv_fraction) >> 8;
                p2->a = 255;
            }

            intery_fp += gradient_fp;
        }
    } else {
        int startX = std::max(0, x0);
        int endX = std::min(width - 1, x1);

        if (startX > x0)
            intery_fp += gradient_fp * (startX - x0);

        for (int x = startX; x <= endX; x++) {
            int y_base = intery_fp >> 16;
            uint8_t fraction = (intery_fp & 0xFFFF) >> 8;
            uint8_t inv_fraction = 255 - fraction;

            if (y_base >= 0 && y_base < height) {
                Color src1 = sampleTexture(ctx, x, y_base);
                Color *p1 = &points.pixels[y_base * width + x];
                p1->r = (src1.r * inv_fraction + p1->r * fraction) >> 8;
                p1->g = (src1.g * inv_fraction + p1->g * fraction) >> 8;
                p1->b = (src1.b * inv_fraction + p1->b * fraction) >> 8;
                p1->a = 255;
            }

            if (y_base + 1 >= 0 && y_base + 1 < height) {
                Color src2 = sampleTexture(ctx, x, y_base + 1);
                Color *p2 = &points.pixels[(y_base + 1) * width + x];
                p2->r = (src2.r * fraction + p2->r * inv_fraction) >> 8;
                p2->g = (src2.g * fraction + p2->g * inv_fraction) >> 8;
                p2->b = (src2.b * fraction + p2->b * inv_fraction) >> 8;
                p2->a = 255;
            }

            intery_fp += gradient_fp;
        }
    }
}

void scanlineFill(Display &displayGrid,
                  const std::vector<std::pair<int, int>> &vertices,
                  const PaintCtx &ctx) {
    if (vertices.size() < 3)
        return;

    int minY = vertices[0].second, maxY = vertices[0].second;
    for (const auto &v : vertices) {
        minY = std::min(minY, v.second);
        maxY = std::max(maxY, v.second);
    }

    minY = std::max(0, minY);
    maxY = std::min(displayGrid.height - 1, maxY);

    uint8_t finalAlpha = ctx.color.a;
    if (finalAlpha == 0)
        return;
    uint32_t invAlpha = 255 - finalAlpha;
    uint32_t r = ctx.color.r;
    uint32_t g = ctx.color.g;
    uint32_t b = ctx.color.b;
    bool hasTexture = (ctx.texture != nullptr);

    std::vector<int> nodes;
    nodes.reserve(16);
    size_t n = vertices.size();

    for (int y = minY; y <= maxY; y++) {
        nodes.clear();

        for (size_t i = 0; i < n; i++) {
            size_t j = (i + 1) % n;
            int xi = vertices[i].first, yi = vertices[i].second;
            int xj = vertices[j].first, yj = vertices[j].second;

            if ((yi < y && yj >= y) || (yj < y && yi >= y))
                nodes.push_back(xi + (y - yi) * (xj - xi) / (yj - yi));
        }

        std::sort(nodes.begin(), nodes.end());

        for (size_t k = 0; k + 1 < nodes.size(); k += 2) {
            int startX = std::max(0, nodes[k]);
            int endX = std::min(displayGrid.width - 1, nodes[k + 1]);

            if (startX > endX)
                continue;

            if (!hasTexture) {
                Color *p = &displayGrid.pixels[y * displayGrid.width + startX];
                for (int x = startX; x <= endX; x++) {
                    p->r = (r * finalAlpha + p->r * invAlpha) >> 8;
                    p->g = (g * finalAlpha + p->g * invAlpha) >> 8;
                    p->b = (b * finalAlpha + p->b * invAlpha) >> 8;
                    p->a = 255;
                    p++;
                }
            } else {
                for (int x = startX; x <= endX; x++)
                    addPixel(displayGrid, x, y, 1.0f, ctx);
            }
        }
    }
}

void scanlineFill(Display &displayGrid,
                  const std::array<std::pair<int, int>, 4> &vertices,
                  const PaintCtx &ctx) {
    int minY = std::min({vertices[0].second, vertices[1].second,
                         vertices[2].second, vertices[3].second});
    int maxY = std::max({vertices[0].second, vertices[1].second,
                         vertices[2].second, vertices[3].second});

    minY = std::max(0, minY);
    maxY = std::min(displayGrid.height - 1, maxY);

    uint8_t finalAlpha = ctx.color.a;
    if (finalAlpha == 0)
        return;
    uint32_t invAlpha = 255 - finalAlpha;
    uint32_t r = ctx.color.r;
    uint32_t g = ctx.color.g;
    uint32_t b = ctx.color.b;
    bool hasTexture = (ctx.texture != nullptr);

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

        Color *targetPixel =
            &displayGrid.pixels[y * displayGrid.width + xStart];

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
            float cur_u = ctx.tex_A * xStart + ctx.tex_B * y + ctx.tex_C;
            float cur_v = ctx.tex_D * xStart + ctx.tex_E * y + ctx.tex_F;

            for (int x = xStart; x <= xEnd; ++x) {
                int u = static_cast<int>(cur_u + 0.5f);
                int v = static_cast<int>(cur_v + 0.5f);

                Color texColor = ctx.texture->sample(u, v);
                uint32_t sampledAlpha = (texColor.a * ctx.color.a) >> 8;
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

                cur_u += ctx.tex_A;
                cur_v += ctx.tex_D;
            }
        }
    }
}
