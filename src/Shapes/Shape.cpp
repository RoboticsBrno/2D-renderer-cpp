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

// Constructor & Destructor
Shape::Shape(const ShapeParams &params)
    : x(params.x), y(params.y), color(params.color), z(params.z),
      parent(nullptr), texture(nullptr), fixTexture(false), collider(nullptr) {
    rotation = {0, 0, 0.0f};
    scale = {1.0f, 1.0f, 0, 0};
    uvTransform = {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
}

Shape::~Shape() = default;

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
void Shape::addCollider(std::unique_ptr<Collider> collider) {
    this->collider = collider ? std::move(collider) : defaultCollider();
}

void Shape::removeCollider() {
    collider.reset();
}

bool Shape::intersects(const std::shared_ptr<Shape> &other) {
    if (this->collider && other && other->collider) {
        return this->collider->intersects(other->collider.get());
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

// Parent-child relationship
void Shape::setParent(Shape *parent) { this->parent = parent; }

PaintCtx Shape::makePaintCtx() const {
    return {color, texture, tex_A, tex_B, tex_C, tex_D, tex_E, tex_F};
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


