#pragma once
#include "Shape.hpp"
#include <vector>

class Collection : public Shape {
  private:
    std::vector<Shape *> shapes;

  public:
    Collection(const ShapeParams &params);
    virtual ~Collection();

    Collider *defaultCollider() override;
    void addShape(Shape *shape);
    std::vector<Shape *> getShapes() { return shapes; }
    Pixels drawAntiAliased() override;
    Pixels drawAliased() override;
};
