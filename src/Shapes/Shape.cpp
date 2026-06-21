#include "Shapes/Shape.hpp"
#include "Shapes/Collection.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>

Matrix2D multiplyMatrices(const Matrix2D &m1, const Matrix2D &m2) {
    Matrix2D out;
    out.a = m1.a * m2.a + m1.c * m2.b;
    out.b = m1.b * m2.a + m1.d * m2.b;
    out.c = m1.a * m2.c + m1.c * m2.d;
    out.d = m1.b * m2.c + m1.d * m2.d;
    out.e = m1.a * m2.e + m1.c * m2.f + m1.e;
    out.f = m1.b * m2.e + m1.d * m2.f + m1.f;
    return out;
}

Matrix2D Shape::getLocalMatrix() {
    updateTrigCache();

    float c = cosAngle;
    float s = sinAngle;

    float px = static_cast<float>(rotation.x);
    float py = static_cast<float>(rotation.y);

    Matrix2D m;
    m.a = c;
    m.b = -s;
    m.c = s;
    m.d = c;

    m.e = this->x + px * (1.0f - c) - py * s;
    m.f = this->y + py * (1.0f - c) + px * s;

    return m;
}

Matrix2D Shape::getGlobalMatrix() {
    if (!isDirty) {
        return cachedGlobalMatrix;
    }

    Matrix2D localMat = getLocalMatrix();

    if (parent != nullptr) {
        cachedGlobalMatrix =
            multiplyMatrices(parent->getGlobalMatrix(), localMat);
    } else {
        cachedGlobalMatrix = localMat;
    }

    isDirty = false;
    return cachedGlobalMatrix;
}

void Shape::getUVAt(int x, int y, float &outU, float &outV) {
    float dx = static_cast<float>(x) - currentScreenPivotX;
    float dy = static_cast<float>(y) - currentScreenPivotY;

    float localX = dx;
    float localY = dy;

    if (fixTexture && rotation.angle != 0) {
        localX = dx * cosAngle - dy * sinAngle;
        localY = dx * sinAngle + dy * cosAngle;
    }

    float localPivotX = static_cast<float>(rotation.x) - this->x;
    float localPivotY = static_cast<float>(rotation.y) - this->y;

    float finalU = localX + localPivotX;
    float finalV = localY + localPivotY;

    float u = finalU * uvTransform.invScaleX + uvTransform.offsetX;
    float v = finalV * uvTransform.invScaleY + uvTransform.offsetY;

    if (uvTransform.rotation != 0) {
        float texTranslatedU = u - 0.5f;
        float texTranslatedV = v - 0.5f;
        u = texTranslatedU * texCos - texTranslatedV * texSin + 0.5f;
        v = texTranslatedU * texSin + texTranslatedV * texCos + 0.5f;
    }

    outU = u;
    outV = v;
}

// Constructor & Destructor
Shape::Shape(const ShapeParams &params)
    : x(params.x), y(params.y), color(params.color), z(params.z),
      parent(nullptr), texture(nullptr), fixTexture(false), collider(nullptr) {
    rotation = {0, 0, 0.0f};
    scale = {1.0f, 1.0f, 0, 0};
    uvTransform = {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
}

Shape::~Shape() { delete collider; }

// Cache management
void Shape::updateTrigCache() {
    if (!trigCacheValid) {
        float angleRad = rotation.angle * M_PI / 180.0f;
        cosAngle = std::cos(angleRad);
        sinAngle = std::sin(angleRad);
        trigCacheValid = true;
    }
}

void Shape::updateTextureTrigCache() {
    if (!texTrigCacheValid) {
        float angleRad = uvTransform.rotation * M_PI / 180.0f;
        texCos = std::cos(angleRad);
        texSin = std::sin(angleRad);
        texTrigCacheValid = true;
    }
}

void Shape::updateTextureMatrix() {
    if (!texture)
        return;

    float invA = 1.0f, invB = 0.0f, invC = 0.0f;
    float invD = 0.0f, invE = 1.0f, invF = 0.0f;

    if (fixTexture) {
        Matrix2D g = getGlobalMatrix();
        float det = g.a * g.d - g.b * g.c;

        if (std::abs(det) > 0.0001f) {
            float invDet = 1.0f / det;
            invA = g.d * invDet;
            invB = -g.c * invDet;
            invC = (g.c * g.f - g.d * g.e) * invDet;

            invD = -g.b * invDet;
            invE = g.a * invDet;
            invF = (g.b * g.e - g.a * g.f) * invDet;
        }
    } else {
        float localPivotX = static_cast<float>(rotation.x) - this->x;
        float localPivotY = static_cast<float>(rotation.y) - this->y;

        invA = 1.0f;
        invB = 0.0f;
        invC = -currentScreenPivotX + localPivotX;

        invD = 0.0f;
        invE = 1.0f;
        invF = -currentScreenPivotY + localPivotY;
    }

    float uA = invA * uvTransform.invScaleX;
    float uB = invB * uvTransform.invScaleX;
    float uC = invC * uvTransform.invScaleX + uvTransform.offsetX;

    float vD = invD * uvTransform.invScaleY;
    float vE = invE * uvTransform.invScaleY;
    float vF = invF * uvTransform.invScaleY + uvTransform.offsetY;

    if (uvTransform.rotation != 0) {
        uC -= 0.5f;
        vF -= 0.5f;

        float tmpA = uA * texCos - vD * texSin;
        float tmpB = uB * texCos - vE * texSin;
        float tmpC = uC * texCos - vF * texSin;

        vD = uA * texSin + vD * texCos;
        vE = uB * texSin + vE * texCos;
        vF = uC * texSin + vF * texCos;

        uA = tmpA;
        uB = tmpB;
        uC = tmpC;

        uC += 0.5f;
        vF += 0.5f;
    }

    tex_A = uA;
    tex_B = uB;
    tex_C = uC;
    tex_D = vD;
    tex_E = vE;
    tex_F = vF;
}
// Collider methods
void Shape::addCollider(Collider *collider) {
    delete this->collider;
    this->collider = collider ? collider : defaultCollider();
}

void Shape::removeCollider() {
    if (collider) {
        delete collider;
        collider = nullptr;
    }
}

bool Shape::intersects(const std::shared_ptr<Shape> &other) {
    if (this->collider && other && other->collider) {
        return this->collider->intersects(other->collider);
    }
    return false;
}

// Position and transformation methods
void Shape::setPosition(int x, int y) {
    this->x = x;
    this->y = y;
    markDirty();
    if (collider) {
        collider->setPosition(x, y);
    }
}

void Shape::translate(int dx, int dy) {
    x += dx;
    y += dy;

    markDirty();
    if (collider) {
        collider->translate(dx, dy);
    }
}

void Shape::translate(float dx, float dy) {
    x += dx;
    y += dy;

    markDirty();
    if (collider) {
        collider->translate(dx, dy);
    }
}

void Shape::rotate(float angle) {
    rotation.angle = std::fmod(rotation.angle + angle, 360.0f);
    invalidateTrigCache();
    markDirty();

    if (this->collider) {
        this->collider->rotate(angle);
    }
}

void Shape::setPivot(int x, int y) {
    rotation.x = x;
    rotation.y = y;

    markDirty();
}

void Shape::setZ(int z) { this->z = z; }

// Scaling methods
void Shape::setScale(float scaleX, float scaleY, float originX, float originY) {
    scale.x = scaleX;
    scale.y = scaleY;
    if (originX != -1)
        scale.originX = originX;
    if (originY != -1)
        scale.originY = originY;

    markDirty();
}

void Shape::scaleX(float scaleX, float originX) {
    scale.x = scaleX;
    markDirty();
    if (originX != -1)
        scale.originX = originX;
}

void Shape::scaleY(float scaleY, float originY) {
    scale.y = scaleY;
    markDirty();
    if (originY != -1)
        scale.originY = originY;
}

void Shape::setScaleOrigin(int x, int y) {
    markDirty();
    scale.originX = x;
    scale.originY = y;
}

// Texture methods
void Shape::setTexture(Texture *texture) { this->texture = texture; }

void Shape::setTextureScale(float scaleX, float scaleY) {
    uvTransform.scaleX = scaleX;
    uvTransform.scaleY = scaleY;
    uvTransform.invScaleX = 1.0f / scaleX;
    uvTransform.invScaleY = 1.0f / scaleY;
}

void Shape::setTextureOffset(float offsetX, float offsetY) {
    uvTransform.offsetX = offsetX;
    uvTransform.offsetY = offsetY;
}

void Shape::setTextureRotation(float rotation) {
    uvTransform.rotation = rotation;
    invalidateTexTrigCache();
}

void Shape::setFixTexture(bool fixed) { fixTexture = fixed; }

Color Shape::sampleTexture(int x, int y) {
    if (!texture)
        return color;

    float u = tex_A * x + tex_B * y + tex_C;
    float v = tex_D * x + tex_E * y + tex_F;

    int texU = static_cast<int>(std::round(u));
    int texV = static_cast<int>(std::round(v));

    Color texColor = texture->sample(texU, texV);

    texColor.a = (texColor.a * color.a) >> 8;

    return texColor;
}

// Parent-child relationship
void Shape::setParent(Shape *parent) { this->parent = parent; }

// Transformation and coordinate methods
void Shape::transformPoint(int x, int y, const Matrix2D &m, int &outX,
                           int &outY) {
    float tx = x * m.a + y * m.c + m.e;
    float ty = x * m.b + y * m.d + m.f;

    outX = static_cast<int>(tx + (tx >= 0.0f ? 0.5f : -0.5f));
    outY = static_cast<int>(ty + (ty >= 0.0f ? 0.5f : -0.5f));
}

std::pair<int, int> Shape::getTransformedPosition(int inputX, int inputY) {
    Matrix2D globalMat = getGlobalMatrix();
    int outX, outY;
    transformPoint(inputX, inputY, globalMat, outX, outY);
    return {outX, outY};
}

void Shape::draw(Display &displayGrid, const DrawOptions &options) {
    float localPivotX = (float)rotation.x - this->x;
    float localPivotY = (float)rotation.y - this->y;
    auto pivotInt = getTransformedPosition(localPivotX, localPivotY);

    currentScreenPivotX = static_cast<float>(pivotInt.first);
    currentScreenPivotY = static_cast<float>(pivotInt.second);

    if (texture) {
        updateTextureMatrix();
    }

    options.antialias ? drawAntiAliased(displayGrid) : drawAliased(displayGrid);
}

// Line drawing algorithms
void Shape::bresenhamLine(Display &displayGrid, int x0, int y0, int x1,
                          int y1) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        addPixel(displayGrid, x0, y0, 1.0f);

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

void Shape::wuLine(Display &points, int x0_int, int y0_int, int x1_int,
                   int y1_int) {
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
            points.pixels[y0_int * points.width + x0_int] = color;
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
                Color src1 = this->sampleTexture(y_base, x);

                Color *p1 = &points.pixels[rowIndex + y_base];
                p1->r = (src1.r * inv_fraction + p1->r * fraction) >> 8;
                p1->g = (src1.g * inv_fraction + p1->g * fraction) >> 8;
                p1->b = (src1.b * inv_fraction + p1->b * fraction) >> 8;
                p1->a = 255;
            }

            if (y_base + 1 >= 0 && y_base + 1 < width) {
                Color src2 = this->sampleTexture(y_base + 1, x);

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

        if (startX > x0) {
            intery_fp += gradient_fp * (startX - x0);
        }

        for (int x = startX; x <= endX; x++) {
            int y_base = intery_fp >> 16;
            uint8_t fraction = (intery_fp & 0xFFFF) >> 8;
            uint8_t inv_fraction = 255 - fraction;

            if (y_base >= 0 && y_base < height) {
                Color src1 = this->sampleTexture(x, y_base);

                Color *p1 = &points.pixels[y_base * width + x];
                p1->r = (src1.r * inv_fraction + p1->r * fraction) >> 8;
                p1->g = (src1.g * inv_fraction + p1->g * fraction) >> 8;
                p1->b = (src1.b * inv_fraction + p1->b * fraction) >> 8;
                p1->a = 255;
            }

            if (y_base + 1 >= 0 && y_base + 1 < height) {
                Color src2 = this->sampleTexture(x, y_base + 1);

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

// Pixel manipulation
void Shape::addPixel(Display &displayGrid, int x, int y, float alpha) {
    if (alpha <= 0.0f)
        return;

    if (static_cast<unsigned>(x) >= static_cast<unsigned>(displayGrid.width) ||
        static_cast<unsigned>(y) >= static_cast<unsigned>(displayGrid.height)) {
        return;
    }

    uint8_t finalAlpha = static_cast<uint8_t>(alpha * color.a);
    if (finalAlpha == 0)
        return;
    uint32_t invAlpha = 255 - finalAlpha;

    int index = y * displayGrid.width + x;
    Color *targetPixel = &displayGrid.pixels[index];

    if (!texture) {
        targetPixel->r =
            (color.r * finalAlpha + targetPixel->r * invAlpha) >> 8;
        targetPixel->g =
            (color.g * finalAlpha + targetPixel->g * invAlpha) >> 8;
        targetPixel->b =
            (color.b * finalAlpha + targetPixel->b * invAlpha) >> 8;
    } else {
        Color texColor = sampleTexture(x, y);
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

// Fill algorithms
void Shape::getInsidePoints(Display &displayGrid,
                            const std::vector<std::pair<int, int>> &vertices) {
    if (vertices.size() < 3)
        return;

    int minY = vertices[0].second;
    int maxY = vertices[0].second;

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

    if (hasTexture) {
        if (fixTexture && rotation.angle != 0)
            updateTrigCache();
        if (uvTransform.rotation != 0)
            updateTextureTrigCache();
    }
    int intersections[16];

    for (int y = minY; y <= maxY; ++y) {
        int count = 0;
        size_t n = vertices.size();

        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            int xi = vertices[i].first, yi = vertices[i].second;
            int xj = vertices[j].first, yj = vertices[j].second;

            if ((yi < y && yj >= y) || (yj < y && yi >= y)) {
                intersections[count++] = xi + (y - yi) * (xj - xi) / (yj - yi);
            }
        }

        if (count == 0)
            continue;

        if (count == 2) {
            if (intersections[0] > intersections[1]) {
                int temp = intersections[0];
                intersections[0] = intersections[1];
                intersections[1] = temp;
            }
        } else if (count > 2) {
            std::sort(intersections, intersections + count);
        }

        for (int i = 0; i + 1 < count; i += 2) {
            int xStart = std::max(0, intersections[i]);
            int xEnd = std::min(displayGrid.width - 1, intersections[i + 1]);

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

                    targetPixel->r = (texColor.r * texAlpha +
                                      targetPixel->r * invTexAlpha) >>
                                     8;
                    targetPixel->g = (texColor.g * texAlpha +
                                      targetPixel->g * invTexAlpha) >>
                                     8;
                    targetPixel->b = (texColor.b * texAlpha +
                                      targetPixel->b * invTexAlpha) >>
                                     8;
                    targetPixel->a = 255;
                    targetPixel++;

                    cur_u += tex_A;
                    cur_v += tex_D;
                }
            }
        }
    }
}
