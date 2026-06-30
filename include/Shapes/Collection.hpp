#pragma once
#include "Shape.hpp"
#include <algorithm>
#include <memory>
#include <vector>

class Collection : public Shape {
  public:
    Collection(const ShapeParams &params);
    ~Collection() override;

    std::unique_ptr<Collider> defaultCollider() override;
    void addShape(std::shared_ptr<Shape> shape);
    void removeShape(std::shared_ptr<Shape> shape);

    void drawAliased(Display &displayGrid) override;
    void drawAntiAliased(Display &displayGrid) override;

    void clear();

    void markDirty() override;

  private:
    std::vector<std::shared_ptr<Shape>> shapes;

    bool needsSort = true;
    std::vector<std::shared_ptr<Shape>> cachedSortedShapes;
};
