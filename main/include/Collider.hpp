#pragma once
#include <cmath>
#include <cstring>
#include <vector>

// Forward declarations
class CircleCollider;
class RectangleCollider;
class PolygonCollider;
class LineSegmentCollider;
class PointCollider;
class RegularPolygonCollider;

// Collider type enum
enum class ColliderType {
    CIRCLE,
    RECTANGLE,
    POLYGON,
    LINE,
    POINT,
    REGULAR_POLYGON
};

// Visitor Interface
class CollisionVisitor {
  public:
    virtual bool visitCircle(const CircleCollider *circle) = 0;
    virtual bool visitRectangle(const RectangleCollider *rect) = 0;
    virtual bool visitPolygon(const PolygonCollider *polygon) = 0;
    virtual bool visitLine(const LineSegmentCollider *line) = 0;
    virtual bool visitPoint(const PointCollider *point) = 0;
    virtual bool
    visitRegularPolygon(const RegularPolygonCollider *regularPolygon) = 0;
    virtual ~CollisionVisitor() = default;
};

// Collidable Interface
class Collidable {
  public:
    virtual bool accept(CollisionVisitor *visitor) const = 0;
    virtual ColliderType getType() const = 0;
    virtual ~Collidable() = default;
};

// Base Collider
class Collider : public Collidable {
  public:
    float x;
    float y;

    Collider(float x, float y) : x(x), y(y) {}
    virtual ~Collider() = default;

    virtual bool intersects(const Collider *other) const = 0;
    virtual void translate(float dx, float dy) {
        x += dx;
        y += dy;
    }
};

// Collision Math utilities
namespace CollisionMath {
inline float distanceSquared(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

inline bool pointInPolygon(float x, float y,
                           const std::vector<std::pair<float, float>> &points) {
    bool inside = false;
    size_t n = points.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        float xi = points[i].first, yi = points[i].second;
        float xj = points[j].first, yj = points[j].second;

        bool intersect = ((yi > y) != (yj > y)) &&
                         (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
        if (intersect)
            inside = !inside;
    }
    return inside;
}

inline bool lineIntersectLine(float x1, float y1, float x2, float y2, float x3,
                              float y3, float x4, float y4) {
    float denominator = ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));

    if (denominator == 0)
        return false;

    float ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denominator;
    float ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denominator;

    return ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1;
}
} // namespace CollisionMath

// Intersection Visitor
class IntersectionVisitor : public CollisionVisitor {
  private:
    const Collider *other;

  public:
    IntersectionVisitor(const Collider *other) : other(other) {}

    bool visitCircle(const CircleCollider *circle) override;
    bool visitRectangle(const RectangleCollider *rect) override;
    bool visitPolygon(const PolygonCollider *polygon) override;
    bool visitLine(const LineSegmentCollider *line) override;
    bool visitPoint(const PointCollider *point) override;
    bool
    visitRegularPolygon(const RegularPolygonCollider *regularPolygon) override;

  private:
    // Intersection implementations
    bool circleCircle(const CircleCollider *c1, const CircleCollider *c2);
    bool circleRectangle(const CircleCollider *circle,
                         const RectangleCollider *rect);
    bool circlePoint(const CircleCollider *circle, const PointCollider *point);
    bool circleLine(const CircleCollider *circle,
                    const LineSegmentCollider *line);
    bool circlePolygon(const CircleCollider *circle,
                       const PolygonCollider *polygon);
    bool circleRegularPolygon(const CircleCollider *circle,
                              const RegularPolygonCollider *regularPolygon);

    bool rectangleRectangle(const RectangleCollider *r1,
                            const RectangleCollider *r2);
    bool rectanglePoint(const RectangleCollider *rect,
                        const PointCollider *point);
    bool rectangleLine(const RectangleCollider *rect,
                       const LineSegmentCollider *line);
    bool rectanglePolygon(const RectangleCollider *rect,
                          const PolygonCollider *polygon);
    bool rectangleRegularPolygon(const RectangleCollider *rect,
                                 const RegularPolygonCollider *regularPolygon);

    bool polygonPoint(const PolygonCollider *polygon,
                      const PointCollider *point);
    bool polygonLine(const PolygonCollider *polygon,
                     const LineSegmentCollider *line);
    bool polygonPolygon(const PolygonCollider *p1, const PolygonCollider *p2);
    bool polygonRegularPolygon(const PolygonCollider *polygon,
                               const RegularPolygonCollider *regularPolygon);

    bool linePoint(const LineSegmentCollider *line, const PointCollider *point);
    bool lineLine(const LineSegmentCollider *l1, const LineSegmentCollider *l2);
    bool lineRegularPolygon(const LineSegmentCollider *line,
                            const RegularPolygonCollider *regularPolygon);

    bool pointPoint(const PointCollider *p1, const PointCollider *p2);
    bool pointRegularPolygon(const PointCollider *point,
                             const RegularPolygonCollider *regularPolygon);
};

// Concrete Collider Classes
class CircleCollider : public Collider {
  public:
    float radius;

    CircleCollider(float x, float y, float radius)
        : Collider(x, y), radius(radius) {}

    bool accept(CollisionVisitor *visitor) const override {
        return visitor->visitCircle(this);
    }

    ColliderType getType() const override { return ColliderType::CIRCLE; }

    bool intersects(const Collider *other) const override {
        IntersectionVisitor visitor(other);
        return this->accept(&visitor);
    }
};

class RectangleCollider : public Collider {
  public:
    float width;
    float height;

    RectangleCollider(float x, float y, float width, float height)
        : Collider(x, y), width(width), height(height) {}

    bool accept(CollisionVisitor *visitor) const override {
        return visitor->visitRectangle(this);
    }

    ColliderType getType() const override { return ColliderType::RECTANGLE; }

    bool intersects(const Collider *other) const override {
        IntersectionVisitor visitor(other);
        return this->accept(&visitor);
    }
};

class PolygonCollider : public Collider {
  public:
    std::vector<std::pair<float, float>> points;

    PolygonCollider(float x, float y,
                    const std::vector<std::pair<float, float>> &points)
        : Collider(x, y), points(points) {}

    bool accept(CollisionVisitor *visitor) const override {
        return visitor->visitPolygon(this);
    }

    ColliderType getType() const override { return ColliderType::POLYGON; }

    bool intersects(const Collider *other) const override {
        IntersectionVisitor visitor(other);
        return this->accept(&visitor);
    }

    std::vector<std::pair<float, float>> getWorldPoints() const {
        std::vector<std::pair<float, float>> worldPoints;
        for (const auto &p : points) {
            worldPoints.push_back({p.first + x, p.second + y});
        }
        return worldPoints;
    }
};

class LineSegmentCollider : public Collider {
  public:
    float x2;
    float y2;

    LineSegmentCollider(float x1, float y1, float x2, float y2)
        : Collider(x1, y1), x2(x2), y2(y2) {}

    bool accept(CollisionVisitor *visitor) const override {
        return visitor->visitLine(this);
    }

    ColliderType getType() const override { return ColliderType::LINE; }

    bool intersects(const Collider *other) const override {
        IntersectionVisitor visitor(other);
        return this->accept(&visitor);
    }
};

class PointCollider : public Collider {
  public:
    PointCollider(float x, float y) : Collider(x, y) {}

    bool accept(CollisionVisitor *visitor) const override {
        return visitor->visitPoint(this);
    }

    ColliderType getType() const override { return ColliderType::POINT; }

    bool intersects(const Collider *other) const override {
        IntersectionVisitor visitor(other);
        return this->accept(&visitor);
    }
};

class RegularPolygonCollider : public Collider {
  public:
    int sides;
    float radius;

    RegularPolygonCollider(float x, float y, int sides, float radius)
        : Collider(x, y), sides(sides), radius(radius) {}

    bool accept(CollisionVisitor *visitor) const override {
        return visitor->visitRegularPolygon(this);
    }

    ColliderType getType() const override {
        return ColliderType::REGULAR_POLYGON;
    }

    bool intersects(const Collider *other) const override {
        IntersectionVisitor visitor(other);
        return this->accept(&visitor);
    }

    std::vector<std::pair<float, float>> generateRegularPolygonPoints() const {
        std::vector<std::pair<float, float>> points;
        for (int i = 0; i < sides; i++) {
            float angle = (i * 2 * M_PI / sides) - M_PI / 2;
            points.push_back(
                {std::cos(angle) * radius, std::sin(angle) * radius});
        }
        return points;
    }
};
