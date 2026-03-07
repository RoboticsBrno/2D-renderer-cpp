#include "Collider.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

// --- Helper Functions ---
static std::vector<std::pair<int, int>>
getRegularPolyVertices(const RegularPolygonCollider *rp) {
    std::vector<std::pair<int, int>> vertices;
    int sides = rp->getSides();

    float rotationRad = rp->getRotation() * CollisionMath::DEG_TO_RAD;

    float c = std::cos(rotationRad);
    float s = std::sin(rotationRad);
    float angleStep = 2.0f * M_PI / sides;

    float startAngle = -M_PI / 2.0f;

    for (int i = 0; i < sides; i++) {
        float angle = i * angleStep + startAngle;

        float localX = rp->radius * std::cos(angle);
        float localY = rp->radius * std::sin(angle);

        int finalX = static_cast<int>(localX * c + localY * s) + rp->x;
        int finalY = static_cast<int>(-localX * s + localY * c) + rp->y;

        vertices.push_back({finalX, finalY});
    }
    return vertices;
}

bool IntersectionVisitor::checkPolygonVsPolygon(
    const std::vector<std::pair<int, int>> &p1,
    const std::vector<std::pair<int, int>> &p2) {
    for (const auto &p : p1) {
        if (CollisionMath::pointInPolygon(p.first, p.second, p2))
            return true;
    }
    for (const auto &p : p2) {
        if (CollisionMath::pointInPolygon(p.first, p.second, p1))
            return true;
    }
    for (size_t i = 0; i < p1.size(); i++) {
        size_t iNext = (i + 1) % p1.size();
        for (size_t j = 0; j < p2.size(); j++) {
            size_t jNext = (j + 1) % p2.size();
            if (CollisionMath::lineIntersectLine(
                    p1[i].first, p1[i].second, p1[iNext].first,
                    p1[iNext].second, p2[j].first, p2[j].second,
                    p2[jNext].first, p2[jNext].second)) {
                return true;
            }
        }
    }
    return false;
}

// --- Visitor Dispatch Implementations ---

bool IntersectionVisitor::visitCircle(const CircleCollider *circle) {
    switch (other->getType()) {
    case ColliderType::CIRCLE:
        return circleCircle(circle, static_cast<const CircleCollider *>(other));
    case ColliderType::RECTANGLE:
        return circleRectangle(circle,
                               static_cast<const RectangleCollider *>(other));
    case ColliderType::POINT:
        return circlePoint(circle, static_cast<const PointCollider *>(other));
    case ColliderType::LINE:
        return circleLine(circle,
                          static_cast<const LineSegmentCollider *>(other));
    case ColliderType::POLYGON:
        return circlePolygon(circle,
                             static_cast<const PolygonCollider *>(other));
    case ColliderType::REGULAR_POLYGON:
        return circleRegularPolygon(
            circle, static_cast<const RegularPolygonCollider *>(other));
    default:
        return false;
    }
}

bool IntersectionVisitor::visitRectangle(const RectangleCollider *rect) {
    switch (other->getType()) {
    case ColliderType::CIRCLE:
        return circleRectangle(static_cast<const CircleCollider *>(other),
                               rect);
    case ColliderType::RECTANGLE:
        return rectangleRectangle(
            rect, static_cast<const RectangleCollider *>(other));
    case ColliderType::POINT:
        return rectanglePoint(rect, static_cast<const PointCollider *>(other));
    case ColliderType::LINE:
        return rectangleLine(rect,
                             static_cast<const LineSegmentCollider *>(other));
    case ColliderType::POLYGON:
        return rectanglePolygon(rect,
                                static_cast<const PolygonCollider *>(other));
    case ColliderType::REGULAR_POLYGON:
        return rectangleRegularPolygon(
            rect, static_cast<const RegularPolygonCollider *>(other));
    default:
        return false;
    }
}

bool IntersectionVisitor::visitPolygon(const PolygonCollider *polygon) {
    switch (other->getType()) {
    case ColliderType::CIRCLE:
        return circlePolygon(static_cast<const CircleCollider *>(other),
                             polygon);
    case ColliderType::RECTANGLE:
        return rectanglePolygon(static_cast<const RectangleCollider *>(other),
                                polygon);
    case ColliderType::POINT:
        return polygonPoint(polygon, static_cast<const PointCollider *>(other));
    case ColliderType::LINE:
        return polygonLine(polygon,
                           static_cast<const LineSegmentCollider *>(other));
    case ColliderType::POLYGON:
        return polygonPolygon(polygon,
                              static_cast<const PolygonCollider *>(other));
    case ColliderType::REGULAR_POLYGON:
        return polygonRegularPolygon(
            polygon, static_cast<const RegularPolygonCollider *>(other));
    default:
        return false;
    }
}

bool IntersectionVisitor::visitLine(const LineSegmentCollider *line) {
    switch (other->getType()) {
    case ColliderType::CIRCLE:
        return circleLine(static_cast<const CircleCollider *>(other), line);
    case ColliderType::RECTANGLE:
        return rectangleLine(static_cast<const RectangleCollider *>(other),
                             line);
    case ColliderType::POINT:
        return linePoint(line, static_cast<const PointCollider *>(other));
    case ColliderType::LINE:
        return lineLine(line, static_cast<const LineSegmentCollider *>(other));
    case ColliderType::POLYGON:
        return polygonLine(static_cast<const PolygonCollider *>(other), line);
    case ColliderType::REGULAR_POLYGON:
        return lineRegularPolygon(
            line, static_cast<const RegularPolygonCollider *>(other));
    default:
        return false;
    }
}

bool IntersectionVisitor::visitPoint(const PointCollider *point) {
    switch (other->getType()) {
    case ColliderType::CIRCLE:
        return circlePoint(static_cast<const CircleCollider *>(other), point);
    case ColliderType::RECTANGLE:
        return rectanglePoint(static_cast<const RectangleCollider *>(other),
                              point);
    case ColliderType::POINT:
        return pointPoint(point, static_cast<const PointCollider *>(other));
    case ColliderType::LINE:
        return linePoint(static_cast<const LineSegmentCollider *>(other),
                         point);
    case ColliderType::POLYGON:
        return polygonPoint(static_cast<const PolygonCollider *>(other), point);
    case ColliderType::REGULAR_POLYGON:
        return pointRegularPolygon(
            point, static_cast<const RegularPolygonCollider *>(other));
    default:
        return false;
    }
}

bool IntersectionVisitor::visitRegularPolygon(
    const RegularPolygonCollider *rp) {
    switch (other->getType()) {
    case ColliderType::CIRCLE:
        return circleRegularPolygon(static_cast<const CircleCollider *>(other),
                                    rp);
    case ColliderType::RECTANGLE:
        return rectangleRegularPolygon(
            static_cast<const RectangleCollider *>(other), rp);
    case ColliderType::POLYGON:
        return polygonRegularPolygon(
            static_cast<const PolygonCollider *>(other), rp);
    case ColliderType::LINE:
        return lineRegularPolygon(
            static_cast<const LineSegmentCollider *>(other), rp);
    case ColliderType::POINT:
        return pointRegularPolygon(static_cast<const PointCollider *>(other),
                                   rp);
    case ColliderType::REGULAR_POLYGON: {
        auto p1 = getRegularPolyVertices(rp);
        auto p2 = getRegularPolyVertices(
            static_cast<const RegularPolygonCollider *>(other));
        return checkPolygonVsPolygon(p1, p2);
    }
    default:
        return false;
    }
}

// --- Specific Collision Implementations ---

bool IntersectionVisitor::circleCircle(const CircleCollider *c1,
                                       const CircleCollider *c2) {
    float distSq = CollisionMath::distanceSquared(c1->x, c1->y, c2->x, c2->y);
    float radSum = c1->radius + c2->radius;
    return distSq < radSum * radSum;
}

bool IntersectionVisitor::circleRectangle(const CircleCollider *circle,
                                          const RectangleCollider *rect) {
    if (rect->rotation == 0.0f) {
        // Fast AABB Check
        float testX = circle->x;
        float testY = circle->y;

        if (circle->x < rect->x)
            testX = rect->x;
        else if (circle->x > rect->x + rect->width)
            testX = rect->x + rect->width;

        if (circle->y < rect->y)
            testY = rect->y;
        else if (circle->y > rect->y + rect->height)
            testY = rect->y + rect->height;

        float distX = circle->x - testX;
        float distY = circle->y - testY;
        return (distX * distX + distY * distY) <=
               (circle->radius * circle->radius);
    } else {
        // Rotated Rectangle -> Treat as Polygon vs Circle
        auto corners = rect->getCorners();
        if (CollisionMath::pointInPolygon(circle->x, circle->y, corners))
            return true;

        for (size_t i = 0; i < corners.size(); i++) {
            size_t j = (i + 1) % corners.size();
            LineSegmentCollider line(corners[i].first, corners[i].second,
                                     corners[j].first, corners[j].second);
            if (circleLine(circle, &line))
                return true;
        }
        return false;
    }
}

bool IntersectionVisitor::circlePoint(const CircleCollider *circle,
                                      const PointCollider *point) {
    return CollisionMath::distanceSquared(circle->x, circle->y, point->x,
                                          point->y) <=
           (circle->radius * circle->radius);
}

bool IntersectionVisitor::circleLine(const CircleCollider *circle,
                                     const LineSegmentCollider *line) {
    auto p2 = line->getP2();
    int x2 = p2.first;
    int y2 = p2.second;

    float lineLenSq = std::pow(x2 - line->x, 2) + std::pow(y2 - line->y, 2);
    if (lineLenSq == 0)
        return circlePoint(circle, new PointCollider(line->x, line->y));
    float t =
        std::max(0.0f, std::min(1.0f, ((circle->x - line->x) * (x2 - line->x) +
                                       (circle->y - line->y) * (y2 - line->y)) /
                                          lineLenSq));
    float closestX = line->x + t * (x2 - line->x);
    float closestY = line->y + t * (y2 - line->y);
    return CollisionMath::distanceSquared(circle->x, circle->y, closestX,
                                          closestY) <
           (circle->radius * circle->radius);
}

bool IntersectionVisitor::circlePolygon(const CircleCollider *circle,
                                        const PolygonCollider *polygon) {
    auto points = polygon->getWorldPoints();
    if (CollisionMath::pointInPolygon(circle->x, circle->y, points))
        return true;

    for (size_t i = 0; i < points.size(); i++) {
        size_t j = (i + 1) % points.size();
        LineSegmentCollider line(points[i].first, points[i].second,
                                 points[j].first, points[j].second);
        if (circleLine(circle, &line))
            return true;
    }
    return false;
}

bool IntersectionVisitor::circleRegularPolygon(
    const CircleCollider *circle, const RegularPolygonCollider *rp) {
    auto points = getRegularPolyVertices(rp);

    if (CollisionMath::pointInPolygon(circle->x, circle->y, points))
        return true;

    for (size_t i = 0; i < points.size(); i++) {
        size_t j = (i + 1) % points.size();
        LineSegmentCollider line(points[i].first, points[i].second,
                                 points[j].first, points[j].second);
        if (circleLine(circle, &line))
            return true;
    }
    return false;
}

bool IntersectionVisitor::rectangleRectangle(const RectangleCollider *r1,
                                             const RectangleCollider *r2) {
    // Optimization: If BOTH are unrotated, use fast AABB
    if (r1->rotation == 0.0f && r2->rotation == 0.0f) {
        return !(r1->x >= r2->x + r2->width || r1->x + r1->width <= r2->x ||
                 r1->y >= r2->y + r2->height || r1->y + r1->height <= r2->y);
    }

    auto p1 = r1->getCorners();
    auto p2 = r2->getCorners();
    return checkPolygonVsPolygon(p1, p2);
}

bool IntersectionVisitor::rectanglePoint(const RectangleCollider *rect,
                                         const PointCollider *point) {
    if (rect->rotation == 0.0f) {
        return point->x >= rect->x && point->x <= rect->x + rect->width &&
               point->y >= rect->y && point->y <= rect->y + rect->height;
    }
    auto corners = rect->getCorners();
    return CollisionMath::pointInPolygon(point->x, point->y, corners);
}

bool IntersectionVisitor::rectangleLine(const RectangleCollider *rect,
                                        const LineSegmentCollider *line) {
    auto corners = rect->getCorners();
    auto p2 = line->getP2();
    int x2 = p2.first;
    int y2 = p2.second;

    if (CollisionMath::pointInPolygon(line->x, line->y, corners) ||
        CollisionMath::pointInPolygon(x2, y2, corners))
        return true;
    for (size_t i = 0; i < corners.size(); i++) {
        size_t j = (i + 1) % corners.size();
        if (CollisionMath::lineIntersectLine(
                line->x, line->y, x2, y2, corners[i].first, corners[i].second,
                corners[j].first, corners[j].second))
            return true;
    }
    return false;
}

bool IntersectionVisitor::rectanglePolygon(const RectangleCollider *rect,
                                           const PolygonCollider *polygon) {
    auto rectPoints = rect->getCorners();
    auto polyPoints = polygon->getWorldPoints();
    return checkPolygonVsPolygon(rectPoints, polyPoints);
}

bool IntersectionVisitor::rectangleRegularPolygon(
    const RectangleCollider *rect, const RegularPolygonCollider *rp) {
    auto rectPoints = rect->getCorners();
    auto polyPoints = getRegularPolyVertices(rp);
    return checkPolygonVsPolygon(rectPoints, polyPoints);
}

bool IntersectionVisitor::polygonPoint(const PolygonCollider *polygon,
                                       const PointCollider *point) {
    auto points = polygon->getWorldPoints();
    return CollisionMath::pointInPolygon(point->x, point->y, points);
}

bool IntersectionVisitor::polygonLine(const PolygonCollider *polygon,
                                      const LineSegmentCollider *line) {
    auto points = polygon->getWorldPoints();
    auto p2 = line->getP2();
    int x2 = p2.first;
    int y2 = p2.second;

    if (CollisionMath::pointInPolygon(line->x, line->y, points) ||
        CollisionMath::pointInPolygon(x2, y2, points))
        return true;
    for (size_t i = 0; i < points.size(); i++) {
        size_t j = (i + 1) % points.size();
        if (CollisionMath::lineIntersectLine(line->x, line->y, x2, y2,
                                             points[i].first, points[i].second,
                                             points[j].first, points[j].second))
            return true;
    }
    return false;
}

bool IntersectionVisitor::polygonPolygon(const PolygonCollider *p1,
                                         const PolygonCollider *p2) {
    auto pts1 = p1->getWorldPoints();
    auto pts2 = p2->getWorldPoints();
    return checkPolygonVsPolygon(pts1, pts2);
}

bool IntersectionVisitor::polygonRegularPolygon(
    const PolygonCollider *polygon, const RegularPolygonCollider *rp) {
    auto pts1 = polygon->getWorldPoints();
    auto pts2 = getRegularPolyVertices(rp);
    return checkPolygonVsPolygon(pts1, pts2);
}

bool IntersectionVisitor::linePoint(const LineSegmentCollider *line,
                                    const PointCollider *point) {
    auto p2 = line->getP2();
    int x2 = p2.first;
    int y2 = p2.second;

    float dist1 = std::sqrt(std::pow(point->x - line->x, 2) +
                            std::pow(point->y - line->y, 2));
    float dist2 =
        std::sqrt(std::pow(point->x - x2, 2) + std::pow(point->y - y2, 2));
    float lineLen =
        std::sqrt(std::pow(line->x - x2, 2) + std::pow(line->y - y2, 2));
    return std::abs((dist1 + dist2) - lineLen) < 0.5f;
}

bool IntersectionVisitor::lineLine(const LineSegmentCollider *l1,
                                   const LineSegmentCollider *l2) {
    auto p1e = l1->getP2();
    auto p2e = l2->getP2();
    return CollisionMath::lineIntersectLine(l1->x, l1->y, p1e.first, p1e.second,
                                            l2->x, l2->y, p2e.first,
                                            p2e.second);
}

bool IntersectionVisitor::lineRegularPolygon(const LineSegmentCollider *line,
                                             const RegularPolygonCollider *rp) {
    auto polyPoints = getRegularPolyVertices(rp);

    auto p2 = line->getP2();
    int x2 = p2.first;
    int y2 = p2.second;

    if (CollisionMath::pointInPolygon(line->x, line->y, polyPoints) ||
        CollisionMath::pointInPolygon(x2, y2, polyPoints))
        return true;
    for (size_t i = 0; i < polyPoints.size(); i++) {
        size_t j = (i + 1) % polyPoints.size();
        if (CollisionMath::lineIntersectLine(
                line->x, line->y, x2, y2, polyPoints[i].first,
                polyPoints[i].second, polyPoints[j].first,
                polyPoints[j].second))
            return true;
    }
    return false;
}

bool IntersectionVisitor::pointPoint(const PointCollider *p1,
                                     const PointCollider *p2) {
    return p1->x == p2->x && p1->y == p2->y;
}

bool IntersectionVisitor::pointRegularPolygon(
    const PointCollider *point, const RegularPolygonCollider *rp) {
    auto points = getRegularPolyVertices(rp);
    return CollisionMath::pointInPolygon(point->x, point->y, points);
}
