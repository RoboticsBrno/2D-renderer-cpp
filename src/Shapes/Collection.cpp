#include "Shapes/Collection.hpp"
#include <memory>

Collection::Collection(const ShapeParams &params) : Shape(params) {}

Collection::~Collection() { clear(); }

std::unique_ptr<Collider> Collection::defaultCollider() {
    return std::make_unique<CircleCollider>(0, 0, 0);
}

void Collection::markDirty() {
    if (isDirty)
        return;

    Shape::markDirty();

    for (std::shared_ptr<Shape> child : shapes) {
        if (child != nullptr) {
            child->markDirty();
        }
    }
}

void Collection::addShape(std::shared_ptr<Shape> shape) {
    if (shape) {
        shape->setParent(this);
        shapes.push_back(shape);
        this->needsSort = true;
    }
}

void Collection::removeShape(std::shared_ptr<Shape> shape) {
    auto it = std::remove(shapes.begin(), shapes.end(), shape);
    if (it != shapes.end()) {
        (*it)->setParent(nullptr);
        shapes.erase(it, shapes.end());
        this->needsSort = true;
    }
}

void Collection::clear() {
    for (auto &shape : shapes) {
        if (shape)
            shape->setParent(nullptr);
    }
    shapes.clear();
    cachedSortedShapes.clear();
    this->needsSort = true;
}

void Collection::drawAliased(Display &displayGrid) {
    if (this->needsSort) {
        this->cachedSortedShapes = shapes;
        std::sort(this->cachedSortedShapes.begin(),
                  this->cachedSortedShapes.end(),
                  [](const std::shared_ptr<Shape> &a,
                     const std::shared_ptr<Shape> &b) {
                      return a->getZ() < b->getZ();
                  });
        this->needsSort = false;
    }

    for (const auto &shape : this->cachedSortedShapes) {
        if (shape)
            shape->drawAliased(displayGrid);
    }
}

void Collection::drawAntiAliased(Display &displayGrid) {
    if (this->needsSort) {
        this->cachedSortedShapes = shapes;
        std::sort(this->cachedSortedShapes.begin(),
                  this->cachedSortedShapes.end(),
                  [](const std::shared_ptr<Shape> &a,
                     const std::shared_ptr<Shape> &b) {
                      return a->getZ() < b->getZ();
                  });
        this->needsSort = false;
    }

    for (const auto &shape : this->cachedSortedShapes) {
        if (shape)
            shape->drawAntiAliased(displayGrid);
    }
}
