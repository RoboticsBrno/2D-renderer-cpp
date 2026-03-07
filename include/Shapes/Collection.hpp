#pragma once
#include "Shape.hpp"
#include <algorithm>
#include <memory>
#include <vector>

class Collection : public Shape {
  public:
    Collection(const ShapeParams &params);
    ~Collection() override;

    Collider *defaultCollider() override;
    void addShape(std::shared_ptr<Shape> shape);
    void removeShape(std::shared_ptr<Shape> shape);

    void drawAliased(Pixels &pixels) override;
    void drawAntiAliased(Pixels &pixels) override;

    void clear();

  private:
    std::vector<std::shared_ptr<Shape>> shapes;

    bool isDirty = true;
    std::vector<std::shared_ptr<Shape>> cachedSortedShapes;
};
