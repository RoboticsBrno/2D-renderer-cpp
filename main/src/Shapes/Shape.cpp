#include "Shape.hpp"
#include <algorithm>
#include <cmath>

Shape::Shape(const ShapeParams &params)
    : x(params.x), y(params.y), color(params.color), z(params.z),
      parent(nullptr), texture(nullptr), fixTexture(false), collider(nullptr) {
    rotation = {params.x, params.y, 0.0f};
    scale = {1.0f, 1.0f, params.x, params.y};
    uvTransform = {1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
}

Shape::~Shape() {
    if (collider) {
        delete collider;
    }
}

void Shape::addCollider(Collider *collider) {
    if (collider) {
        this->collider = collider;
        return;
    }
    this->collider = defaultCollider();
}

void Shape::removeCollider() {
    if (collider) {
        delete collider;
        collider = nullptr;
    }
}

bool Shape::intersects(Shape *other) {
    if (this->collider && other->collider) {
        return this->collider->intersects(other->collider);
    }
    return false;
}

void Shape::translate(float dx, float dy) {
    x += dx;
    y += dy;
    if (collider) {
        collider->translate(dx, dy);
    }
}

void Shape::setTexture(Texture *texture) { this->texture = texture; }

void Shape::setTextureScale(float scaleX, float scaleY) {
    uvTransform.scaleX = scaleX;
    uvTransform.scaleY = scaleY;
}

void Shape::setTextureOffset(float offsetX, float offsetY) {
    uvTransform.offsetX = offsetX;
    uvTransform.offsetY = offsetY;
}

void Shape::setFixTexture(bool fixed) { fixTexture = fixed; }

void Shape::setTextureRotation(float rotation) {
    uvTransform.rotation = rotation;
}

Color Shape::sampleTexture(float x, float y) {
    if (!texture) {
        return color;
    }

    float localX = x - this->x;
    float localY = y - this->y;

    if (fixTexture && rotation.angle != 0) {
        float angleRad = -rotation.angle * M_PI / 180.0f;
        float cos = std::cos(angleRad);
        float sin = std::sin(angleRad);

        float rotationOriginX = rotation.x;
        float rotationOriginY = rotation.y;

        float translatedX = localX - (rotationOriginX - this->x);
        float translatedY = localY - (rotationOriginY - this->y);

        localX = translatedX * cos + translatedY * sin;
        localY = -translatedX * sin + translatedY * cos;

        localX += (rotationOriginX - this->x);
        localY += (rotationOriginY - this->y);
    }

    float u = (localX / uvTransform.scaleX + uvTransform.offsetX);
    float v = (localY / uvTransform.scaleY + uvTransform.offsetY);

    if (uvTransform.rotation != 0) {
        float texAngleRad = uvTransform.rotation * M_PI / 180.0f;
        float texCos = std::cos(texAngleRad);
        float texSin = std::sin(texAngleRad);

        float texCenterX = 0.5f;
        float texCenterY = 0.5f;

        float texTranslatedU = u - texCenterX;
        float texTranslatedV = v - texCenterY;

        u = texTranslatedU * texCos - texTranslatedV * texSin + texCenterX;
        v = texTranslatedU * texSin + texTranslatedV * texCos + texCenterY;
    }

    Color texColor = texture->sample(u, v);

    return Color(static_cast<uint8_t>((color.r * texColor.r) / 255),
                 static_cast<uint8_t>((color.g * texColor.g) / 255),
                 static_cast<uint8_t>((color.b * texColor.b) / 255),
                 color.a * texColor.a);
}

void Shape::setParent(Shape *parent) { this->parent = parent; }

std::pair<float, float> Shape::getTransformedPosition(float x, float y) {
    float currentX = x;
    float currentY = y;

    float scaleOriginX = scale.originX;
    float scaleOriginY = scale.originY;

    currentX = (currentX - scaleOriginX) * scale.x + scaleOriginX;
    currentY = (currentY - scaleOriginY) * scale.y + scaleOriginY;

    float angleRad = rotation.angle * M_PI / 180.0f;
    float cos = std::cos(angleRad);
    float sin = std::sin(angleRad);

    float rotationOriginX = rotation.x;
    float rotationOriginY = rotation.y;

    float translatedX = currentX - rotationOriginX;
    float translatedY = currentY - rotationOriginY;

    currentX = translatedX * cos + translatedY * sin + rotationOriginX;
    currentY = -translatedX * sin + translatedY * cos + rotationOriginY;

    if (parent) {
        auto parentTransformed =
            parent->getTransformedPosition(currentX, currentY);
        currentX = parentTransformed.first;
        currentY = parentTransformed.second;
    }

    return {std::round(currentX), std::round(currentY)};
}

Pixels Shape::bresenhamLine(float x0, float y0, float x1, float y1) {
    float dx = std::abs(x1 - x0);
    float dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    Pixels points;
    float x = x0;
    float y = y0;
    float err = dx - dy;

    points.push_back(
        Pixel(static_cast<int>(x), static_cast<int>(y), sampleTexture(x, y)));

    while (x != x1 || y != y1) {
        float e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }

        if (e2 < dx) {
            err += dx;
            y += sy;
        }

        points.push_back(Pixel(static_cast<int>(x), static_cast<int>(y),
                               sampleTexture(x, y)));
    }

    return points;
}

Pixels Shape::wuLine(float x0, float y0, float x1, float y1) {
    Pixels points;

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = (dx == 0) ? 1.0f : dy / dx;

    auto rfpart = [](float x) { return 1.0f - (x - std::floor(x)); };
    auto fpart = [](float x) { return x - std::floor(x); };

    float xend = std::round(x0);
    float yend = y0 + gradient * (xend - x0);
    float xgap = rfpart(x0 + 0.5f);

    float xpxl1 = xend;
    float ypxl1 = std::floor(yend);

    if (steep) {
        addPixel(points, ypxl1, xpxl1, rfpart(yend) * xgap, true);
        addPixel(points, ypxl1 + 1, xpxl1, fpart(yend) * xgap, true);
    } else {
        addPixel(points, xpxl1, ypxl1, rfpart(yend) * xgap, true);
        addPixel(points, xpxl1, ypxl1 + 1, fpart(yend) * xgap, true);
    }

    float intery = yend + gradient;

    xend = std::round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    float xpxl2 = xend;
    float ypxl2 = std::floor(yend);

    if (steep) {
        addPixel(points, ypxl2, xpxl2, rfpart(yend) * xgap, true);
        addPixel(points, ypxl2 + 1, xpxl2, fpart(yend) * xgap, true);
    } else {
        addPixel(points, xpxl2, ypxl2, rfpart(yend) * xgap, true);
        addPixel(points, xpxl2, ypxl2 + 1, fpart(yend) * xgap, true);
    }

    if (steep) {
        for (float x = xpxl1 + 1; x < xpxl2; x++) {
            addPixel(points, std::floor(intery), x, rfpart(intery));
            addPixel(points, std::floor(intery) + 1, x, fpart(intery));
            intery += gradient;
        }
    } else {
        for (float x = xpxl1 + 1; x < xpxl2; x++) {
            addPixel(points, x, std::floor(intery), rfpart(intery));
            addPixel(points, x, std::floor(intery) + 1, fpart(intery));
            intery += gradient;
        }
    }

    return points;
}

void Shape::addPixel(Pixels &points, float x, float y, float alpha,
                     bool isEndpoint) {
    int clampedX = static_cast<int>(std::max(0.0f, std::round(x)));
    int clampedY = static_cast<int>(std::max(0.0f, std::round(y)));

    float finalAlpha =
        isEndpoint ? 1.0f : std::max(0.0f, std::min(1.0f, alpha));

    if (alpha > 0.01f) {
        Color pixelColor;

        if (!texture) {
            pixelColor =
                Color(color.r, color.g, color.b,
                      std::max(0.0f, std::min(1.0f, finalAlpha * color.a)));
        } else {
            Color texColor = sampleTexture(clampedX, clampedY);
            pixelColor =
                Color(texColor.r, texColor.g, texColor.b,
                      std::max(0.0f, std::min(1.0f, finalAlpha * texColor.a)));
        }

        points.push_back(Pixel(clampedX, clampedY, pixelColor));
    }
}

void Shape::setScale(float scaleX, float scaleY, float originX, float originY) {
    scale.x = scaleX;
    scale.y = scaleY;
    if (originX != -1)
        scale.originX = originX;
    if (originY != -1)
        scale.originY = originY;
}

void Shape::scaleX(float scaleX, float originX) {
    scale.x = scaleX;
    if (originX != -1)
        scale.originX = originX;
}

void Shape::scaleY(float scaleY, float originY) {
    scale.y = scaleY;
    if (originY != -1)
        scale.originY = originY;
}

void Shape::setScaleOrigin(float x, float y) {
    scale.originX = x;
    scale.originY = y;
}

void Shape::changeColor(const Color &color) { this->color = color; }

void Shape::setZ(float z) { this->z = z; }

void Shape::setPosition(float x, float y) {
    this->x = x;
    this->y = y;
}

void Shape::rotate(float angle) {
    rotation.angle = std::fmod(rotation.angle + angle, 360.0f);
}

void Shape::setPivot(float x, float y) {
    rotation.x = x;
    rotation.y = y;
}

Pixels Shape::draw(const DrawOptions &options) {
    return options.antialias ? drawAntiAliased() : drawAliased();
}

Pixels
Shape::getInsidePoints(const std::vector<std::pair<float, float>> &vertices) {
    if (vertices.empty())
        return Pixels();

    float minX = vertices[0].first;
    float maxX = vertices[0].first;
    float minY = vertices[0].second;
    float maxY = vertices[0].second;

    for (const auto &v : vertices) {
        minX = std::min(minX, v.first);
        maxX = std::max(maxX, v.first);
        minY = std::min(minY, v.second);
        maxY = std::max(maxY, v.second);
    }

    Pixels points;
    for (int x = static_cast<int>(minX); x <= static_cast<int>(maxX); x++) {
        for (int y = static_cast<int>(minY); y <= static_cast<int>(maxY); y++) {
            bool inside = false;
            size_t n = vertices.size();
            for (size_t i = 0, j = n - 1; i < n; j = i++) {
                float xi = vertices[i].first, yi = vertices[i].second;
                float xj = vertices[j].first, yj = vertices[j].second;

                bool intersect = ((yi > y) != (yj > y)) &&
                                 (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
                if (intersect)
                    inside = !inside;
            }

            if (inside) {
                points.push_back(Pixel(x, y, color));
            }
        }
    }

    return points;
}
