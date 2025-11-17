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
  protected:
    int x;
    int y;
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
    void translate(int dx, int dy);

    void setTexture(Texture *texture);
    void setTextureScale(float scaleX, float scaleY);
    void setTextureOffset(float offsetX, float offsetY);
    void setFixTexture(bool fixed);
    void setTextureRotation(float rotation);
    Color sampleTexture(int x, int y);

    void setParent(Shape *parent);

    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    const Color &getColor() const { return color; }

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
    void changeColor(const Color &color);
    void setZ(int z);
    void setPosition(int x, int y);
    void rotate(float angle);
    void setPivot(int x, int y);

    void draw(Pixels &pixels, const DrawOptions &options);
    virtual void drawAntiAliased(Pixels &pixels) = 0;
    virtual void drawAliased(Pixels &pixels) = 0;

  protected:
    void getInsidePoints(Pixels &points,
                         const std::vector<std::pair<int, int>> &vertices);
};
