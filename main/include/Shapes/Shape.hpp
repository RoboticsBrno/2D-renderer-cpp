#pragma once
#include "../Collider.hpp"
#include "../Texture.hpp"
#include "../Utils.hpp"
#include <vector>

struct DrawOptions {
    int screen_width;
    int screen_height;
    bool antialias;
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
    float cosAngle = 1.0f, sinAngle = 0.0f;
    float texCos = 1.0f, texSin = 0.0f;
    bool trigCacheValid = false;
    bool texTrigCacheValid = false;

    void updateTrigCache();
    void updateTextureTrigCache();

  protected:
    float x;
    float y;
    Color color;

    struct Rotation {
        int x;
        int y;
        float angle;
    } rotation;

    int z;
    Shape *parent;

    struct Scale {
        float x;
        float y;
        int originX;
        int originY;
    } scale;

    Texture *texture;
    bool fixTexture;

    struct UVTransform {
        float scaleX;
        float scaleY;
        float invScaleX;
        float invScaleY;
        float offsetX;
        float offsetY;
        float rotation;
    } uvTransform;

    Collider *collider;

  public:
    Shape(const ShapeParams &params);
    virtual ~Shape();

    void addCollider(Collider *collider = nullptr);
    Collider *getCollider() const { return collider; }
    virtual Collider *defaultCollider() = 0;
    void removeCollider();
    bool intersects(Shape *other);
    void translate(int dx, int dy);
    void translate(float dx, float dy);

    void setTexture(Texture *texture);
    Texture *getTexture() const { return texture; }
    void setTextureScale(float scaleX, float scaleY);
    void setTextureOffset(float offsetX, float offsetY);
    void setFixTexture(bool fixed);
    bool getFixTexture() const { return fixTexture; }
    void setTextureRotation(float rotation);
    Color sampleTexture(int x, int y);

    float getUVScaleX() const { return uvTransform.scaleX; }
    float getUVScaleY() const { return uvTransform.scaleY; }
    float getUVOffsetX() const { return uvTransform.offsetX; }
    float getUVOffsetY() const { return uvTransform.offsetY; }
    float getUVRotation() const { return uvTransform.rotation; }

    void setUVScaleX(float scaleX) {
        this->uvTransform.scaleX = scaleX;
        this->uvTransform.invScaleX = 1.0f / scaleX;
    }
    void setUVScaleY(float scaleY) {
        this->uvTransform.scaleY = scaleY;
        this->uvTransform.invScaleY = 1.0f / scaleY;
    }
    void setUVOffsetX(float offsetX) { this->uvTransform.offsetX = offsetX; }
    void setUVOffsetY(float offsetY) { this->uvTransform.offsetY = offsetY; }
    void setUVRotation(float rotation) {
        this->uvTransform.rotation = rotation;
        invalidateTexTrigCache();
    }

    void setParent(Shape *parent);
    Shape *getParent() const { return parent; }

    void invalidateTrigCache() { trigCacheValid = false; }
    void invalidateTexTrigCache() { texTrigCacheValid = false; }

    float getX() const { return x; }
    float getY() const { return y; }
    int getZ() const { return z; }
    const Color &getColor() const { return color; }

    void setX(int x) {
        this->x = x;
        if (collider) {
            collider->setX(x);
        }
    }
    void setY(int y) {
        this->y = y;
        if (collider) {
            collider->setY(y);
        }
    }
    void setColor(const Color &color) { this->color = color; }

  protected:
    std::pair<int, int> getTransformedPosition(int x, int y);

    void bresenhamLine(Pixels &points, int x0, int y0, int x1, int y1);
    void wuLine(Pixels &points, int x0, int y0, int x1, int y1);
    void addPixel(Pixels &points, int x, int y, float alpha);

  public:
    void setScale(float scaleX, float scaleY, float originX = -1,
                  float originY = -1);
    void scaleX(float scaleX, float originX = -1);
    void scaleY(float scaleY, float originY = -1);
    void setScaleOrigin(int x, int y);

    float getScaleX() const { return scale.x; }
    float getScaleY() const { return scale.y; }
    int getScaleOriginX() const { return scale.originX; }
    int getScaleOriginY() const { return scale.originY; }

    void setScaleX(float scaleX) { this->scale.x = scaleX; }
    void setScaleY(float scaleY) { this->scale.y = scaleY; }
    void setScaleOriginX(int x) { this->scale.originX = x; }
    void setScaleOriginY(int y) { this->scale.originY = y; }
    void changeColor(const Color &color);
    void setZ(int z);
    void setPosition(int x, int y);
    void rotate(float angle);
    void setPivot(int x, int y);

    float getRotationAngle() const { return rotation.angle; }
    int getRotationX() const { return rotation.x; }
    int getRotationY() const { return rotation.y; }

    void setRotationAngle(float angle) { this->rotation.angle = angle; }
    void setRotationX(int x) { this->rotation.x = x; }
    void setRotationY(int y) { this->rotation.y = y; }

    void draw(Pixels &pixels, const DrawOptions &options);
    virtual void drawAntiAliased(Pixels &pixels) = 0;
    virtual void drawAliased(Pixels &pixels) = 0;

  protected:
    void getInsidePoints(Pixels &points,
                         const std::vector<std::pair<int, int>> &vertices);
};
