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

Matrix2D Shape::localMatrix() {
    updateTrigCache();

    float c = _cosAngle;
    float s = _sinAngle;

    float px = static_cast<float>(_rotation.x);
    float py = static_cast<float>(_rotation.y);

    Matrix2D m;
    m.a = c;
    m.b = -s;
    m.c = s;
    m.d = c;

    m.e = _x + px * (1.0f - c) - py * s;
    m.f = _y + py * (1.0f - c) + px * s;

    return m;
}

Matrix2D Shape::globalMatrix() {
    if (!_isDirty)
        return _cachedGlobalMatrix;

    Matrix2D localMat = localMatrix();

    if (_parent != nullptr) {
        _cachedGlobalMatrix =
            multiplyMatrices(_parent->globalMatrix(), localMat);
    } else {
        _cachedGlobalMatrix = localMat;
    }

    _isDirty = false;
    return _cachedGlobalMatrix;
}

Shape::Shape(const ShapeParams &params)
    : _x(params.x), _y(params.y), _color(params.color), _z(params.z),
      _parent(nullptr), _texture(nullptr), _fixTexture(false),
      _collider(nullptr) {
    _rotation = {0, 0, 0.0f};
    _scale = {1.0f, 1.0f, 0, 0};
    _uvTransform = {1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
}

Shape::~Shape() = default;

void Shape::updateTrigCache() {
    if (!_trigCacheValid) {
        float angleRad = _rotation.angle * M_PI / 180.0f;
        _cosAngle = std::cos(angleRad);
        _sinAngle = std::sin(angleRad);
        _trigCacheValid = true;
    }
}

void Shape::updateTextureTrigCache() {
    if (!_texTrigCacheValid) {
        float angleRad = _uvTransform.rotation * M_PI / 180.0f;
        _texCos = std::cos(angleRad);
        _texSin = std::sin(angleRad);
        _texTrigCacheValid = true;
    }
}

void Shape::updateTextureMatrix() {
    if (!_texture)
        return;

    float invA = 1.0f, invB = 0.0f, invC = 0.0f;
    float invD = 0.0f, invE = 1.0f, invF = 0.0f;

    if (_fixTexture) {
        Matrix2D g = globalMatrix();
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
        float localPivotX = static_cast<float>(_rotation.x) - _x;
        float localPivotY = static_cast<float>(_rotation.y) - _y;

        invA = 1.0f;
        invB = 0.0f;
        invC = -_currentScreenPivotX + localPivotX;

        invD = 0.0f;
        invE = 1.0f;
        invF = -_currentScreenPivotY + localPivotY;
    }

    float uA = invA * _uvTransform.invScaleX;
    float uB = invB * _uvTransform.invScaleX;
    float uC = invC * _uvTransform.invScaleX + _uvTransform.offsetX;

    float vD = invD * _uvTransform.invScaleY;
    float vE = invE * _uvTransform.invScaleY;
    float vF = invF * _uvTransform.invScaleY + _uvTransform.offsetY;

    if (_uvTransform.rotation != 0) {
        uC -= 0.5f;
        vF -= 0.5f;

        float tmpA = uA * _texCos - vD * _texSin;
        float tmpB = uB * _texCos - vE * _texSin;
        float tmpC = uC * _texCos - vF * _texSin;

        vD = uA * _texSin + vD * _texCos;
        vE = uB * _texSin + vE * _texCos;
        vF = uC * _texSin + vF * _texCos;

        uA = tmpA;
        uB = tmpB;
        uC = tmpC;

        uC += 0.5f;
        vF += 0.5f;
    }

    _tex_A = uA;
    _tex_B = uB;
    _tex_C = uC;
    _tex_D = vD;
    _tex_E = vE;
    _tex_F = vF;
}

void Shape::addCollider(std::unique_ptr<Collider> collider) {
    _collider = collider ? std::move(collider) : defaultCollider();
}

void Shape::removeCollider() {
    _collider.reset();
}

bool Shape::intersects(const std::shared_ptr<Shape> &other) {
    if (_collider && other && other->_collider) {
        return _collider->intersects(other->_collider.get());
    }
    return false;
}

void Shape::setPosition(int x, int y) {
    _x = x;
    _y = y;
    markDirty();
    if (_collider)
        _collider->setPosition(x, y);
}

void Shape::translate(int dx, int dy) {
    _x += dx;
    _y += dy;
    markDirty();
    if (_collider)
        _collider->translate(dx, dy);
}

void Shape::translate(float dx, float dy) {
    _x += dx;
    _y += dy;
    markDirty();
    if (_collider)
        _collider->translate(dx, dy);
}

void Shape::rotate(float angle) {
    _rotation.angle = std::fmod(_rotation.angle + angle, 360.0f);
    invalidateTrigCache();
    markDirty();
    if (_collider)
        _collider->rotate(angle);
}

void Shape::setPivot(int x, int y) {
    _rotation.x = x;
    _rotation.y = y;
    markDirty();
}

void Shape::setZ(int z) { _z = z; }

void Shape::setScale(float scaleX, float scaleY, float originX, float originY) {
    _scale.x = scaleX;
    _scale.y = scaleY;
    if (originX != -1)
        _scale.originX = originX;
    if (originY != -1)
        _scale.originY = originY;
    markDirty();
}

void Shape::scaleX(float scaleX, float originX) {
    _scale.x = scaleX;
    markDirty();
    if (originX != -1)
        _scale.originX = originX;
}

void Shape::scaleY(float scaleY, float originY) {
    _scale.y = scaleY;
    markDirty();
    if (originY != -1)
        _scale.originY = originY;
}

void Shape::setScaleOrigin(int x, int y) {
    markDirty();
    _scale.originX = x;
    _scale.originY = y;
}

void Shape::setTexture(Texture *texture) { _texture = texture; }

void Shape::setTextureScale(float scaleX, float scaleY) {
    _uvTransform.scaleX = scaleX;
    _uvTransform.scaleY = scaleY;
    _uvTransform.invScaleX = 1.0f / scaleX;
    _uvTransform.invScaleY = 1.0f / scaleY;
}

void Shape::setTextureOffset(float offsetX, float offsetY) {
    _uvTransform.offsetX = offsetX;
    _uvTransform.offsetY = offsetY;
}

void Shape::setTextureRotation(float rotation) {
    _uvTransform.rotation = rotation;
    invalidateTexTrigCache();
}

void Shape::setFixTexture(bool fixed) { _fixTexture = fixed; }

void Shape::setParent(Shape *parent) { _parent = parent; }

PaintCtx Shape::makePaintCtx() const {
    return {_color, _texture, _tex_A, _tex_B, _tex_C, _tex_D, _tex_E, _tex_F};
}

std::pair<int, int> Shape::getTransformedPosition(int inputX, int inputY) {
    Matrix2D mat = globalMatrix();
    int outX, outY;
    transformPoint(inputX, inputY, mat, outX, outY);
    return {outX, outY};
}

void Shape::draw(Display &displayGrid, const DrawOptions &options) {
    float localPivotX = (float)_rotation.x - _x;
    float localPivotY = (float)_rotation.y - _y;
    auto pivotInt = getTransformedPosition(localPivotX, localPivotY);

    _currentScreenPivotX = static_cast<float>(pivotInt.first);
    _currentScreenPivotY = static_cast<float>(pivotInt.second);

    if (_texture)
        updateTextureMatrix();

    options.antialias ? drawAntiAliased(displayGrid) : drawAliased(displayGrid);
}
