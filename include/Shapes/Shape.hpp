#pragma once
#include "../Collider.hpp"
#include "../DrawUtils.hpp"
#include "../Texture.hpp"
#include "Utils.hpp"
#include <memory>
#include <vector>

struct DrawOptions {
    int screen_width;
    int screen_height;
    bool antialias;
};

struct Matrix2D {
    float a = 1.0f, b = 0.0f; // X-Axis (Scale/Rotate)
    float c = 0.0f, d = 1.0f; // Y-Axis (Skew/Rotate)
    float e = 0.0f, f = 0.0f; // Translation (Position)
};

struct ShapeParams {
    float x;
    float y;
    Color color;
    float z;

    ShapeParams(float x, float y, const Color &color, float z = 0)
        : x(x), y(y), color(color), z(z) {}
};

class Shape {
  private:
    float _cosAngle = 1.0f, _sinAngle = 0.0f;
    float _texCos = 1.0f, _texSin = 0.0f;
    bool _trigCacheValid = false;
    bool _texTrigCacheValid = false;

  protected:
    float _x;
    float _y;
    Color _color;

    float _currentScreenPivotX = 0.0f;
    float _currentScreenPivotY = 0.0f;

    struct Rotation {
        int x;
        int y;
        float angle;
    } _rotation;

    int _z;
    Shape *_parent;

    struct Scale {
        float x;
        float y;
        int originX;
        int originY;
    } _scale;

    Texture *_texture;
    bool _fixTexture;

    struct UVTransform {
        float scaleX;
        float scaleY;
        float invScaleX;
        float invScaleY;
        float offsetX;
        float offsetY;
        float rotation;
    } _uvTransform;

    Matrix2D _cachedGlobalMatrix;
    bool _isDirty = true;

    std::unique_ptr<Collider> _collider;

    std::pair<int, int> getTransformedPosition(int inputX, int inputY);
    PaintCtx makePaintCtx() const;

    void updateTrigCache();
    void updateTextureTrigCache();

    float _tex_A, _tex_B, _tex_C;
    float _tex_D, _tex_E, _tex_F;

    void updateTextureMatrix();

  public:
    Shape(const ShapeParams &params);
    virtual ~Shape();

    Shape(const Shape &) = delete;
    Shape &operator=(const Shape &) = delete;

    Matrix2D localMatrix();
    Matrix2D globalMatrix();

    virtual void drawAntiAliased(Display &pixels) = 0;
    virtual void drawAliased(Display &pixels) = 0;
    virtual std::unique_ptr<Collider> defaultCollider() = 0;

    virtual void markDirty() { _isDirty = true; };

    void draw(Display &pixels, const DrawOptions &options);

    // Transformation
    void setPosition(int x, int y);
    void translate(int dx, int dy);
    void translate(float dx, float dy);
    void rotate(float angle);
    void setPivot(int x, int y);

    void setScale(float scaleX, float scaleY, float originX = -1,
                  float originY = -1);
    void scaleX(float scaleX, float originX = -1);
    void scaleY(float scaleY, float originY = -1);
    void setScaleOrigin(int x, int y);

    // Collider
    void addCollider(std::unique_ptr<Collider> collider = nullptr);
    void removeCollider();
    bool intersects(const std::shared_ptr<Shape> &other);

    // Texture
    void setTexture(Texture *texture);
    void setTextureScale(float scaleX, float scaleY);
    void setTextureOffset(float offsetX, float offsetY);
    void setTextureRotation(float rotation);
    void setFixTexture(bool fixed);

    void setParent(Shape *parent);

    void invalidateTrigCache() { _trigCacheValid = false; }
    void invalidateTexTrigCache() { _texTrigCacheValid = false; }

    // Accessors
    float x() const { return _x; }
    float y() const { return _y; }
    int z() const { return _z; }
    const Color &color() const { return _color; }

    float rotationAngle() const { return _rotation.angle; }
    int rotationX() const { return _rotation.x; }
    int rotationY() const { return _rotation.y; }

    float scaleX() const { return _scale.x; }
    float scaleY() const { return _scale.y; }
    int scaleOriginX() const { return _scale.originX; }
    int scaleOriginY() const { return _scale.originY; }

    Texture *texture() const { return _texture; }
    bool fixTexture() const { return _fixTexture; }

    float uvScaleX() const { return _uvTransform.scaleX; }
    float uvScaleY() const { return _uvTransform.scaleY; }
    float uvOffsetX() const { return _uvTransform.offsetX; }
    float uvOffsetY() const { return _uvTransform.offsetY; }
    float uvRotation() const { return _uvTransform.rotation; }

    Collider *collider() const { return _collider.get(); }
    Shape *parent() const { return _parent; }

    // Setters
    void setX(int x) {
        _x = x;
        if (_collider)
            _collider->setX(x);
    }

    void setY(int y) {
        _y = y;
        if (_collider)
            _collider->setY(y);
    }

    void setColor(const Color &color) { _color = color; }
    void setZ(int z);

    void setRotationAngle(float angle) {
        _rotation.angle = angle;
        if (_collider)
            _collider->setRotation(angle);
    }

    void setRotationX(int x) { _rotation.x = x; }
    void setRotationY(int y) { _rotation.y = y; }

    void setScaleX(float scaleX) { _scale.x = scaleX; }
    void setScaleY(float scaleY) { _scale.y = scaleY; }
    void setScaleOriginX(int x) { _scale.originX = x; }
    void setScaleOriginY(int y) { _scale.originY = y; }

    void setUVScaleX(float scaleX) {
        _uvTransform.scaleX = scaleX;
        _uvTransform.invScaleX = 1.0f / scaleX;
    }

    void setUVScaleY(float scaleY) {
        _uvTransform.scaleY = scaleY;
        _uvTransform.invScaleY = 1.0f / scaleY;
    }

    void setUVOffsetX(float offsetX) { _uvTransform.offsetX = offsetX; }
    void setUVOffsetY(float offsetY) { _uvTransform.offsetY = offsetY; }
    void setUVRotation(float rotation) {
        _uvTransform.rotation = rotation;
        invalidateTexTrigCache();
    }
};
