#pragma once
#include "../Collider.hpp"
#include "../Texture.hpp"
#include "../Utils.hpp"
#include <cmath>
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
  protected:
    float x;
    float y;
    Color color;

    struct Rotation {
        float x;
        float y;
        float angle;
    } rotation;

    float z;
    Shape *parent;

    struct Scale {
        float x;
        float y;
        float originX;
        float originY;
    } scale;

    Texture *texture;
    bool fixTexture;

    struct UVTransform {
        float scaleX;
        float scaleY;
        float offsetX;
        float offsetY;
        float rotation;
    } uvTransform;

    Collider *collider;

  public:
    Shape(const ShapeParams &params);
    virtual ~Shape();

    void addCollider(Collider *collider = nullptr);
    virtual Collider *defaultCollider() = 0;
    void removeCollider();
    bool intersects(Shape *other);
    void translate(float dx, float dy);

    void setTexture(Texture *texture);
    void setTextureScale(float scaleX, float scaleY);
    void setTextureOffset(float offsetX, float offsetY);
    void setFixTexture(bool fixed);
    void setTextureRotation(float rotation);
    Color sampleTexture(float x, float y);

    void setParent(Shape *parent);

    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    const Color &getColor() const { return color; }

  protected:
    std::pair<float, float> getTransformedPosition(float x, float y);

    Pixels bresenhamLine(float x0, float y0, float x1, float y1);
    Pixels wuLine(float x0, float y0, float x1, float y1);
    void addPixel(Pixels &points, float x, float y, float alpha,
                  bool isEndpoint = false);

  public:
    void setScale(float scaleX, float scaleY, float originX = -1,
                  float originY = -1);
    void scaleX(float scaleX, float originX = -1);
    void scaleY(float scaleY, float originY = -1);
    void setScaleOrigin(float x, float y);
    void changeColor(const Color &color);
    void setZ(float z);
    void setPosition(float x, float y);
    void rotate(float angle);
    void setPivot(float x, float y);

    Pixels draw(const DrawOptions &options);
    virtual Pixels drawAntiAliased() = 0;
    virtual Pixels drawAliased() = 0;

  protected:
    Pixels
    getInsidePoints(const std::vector<std::pair<float, float>> &vertices);
};
