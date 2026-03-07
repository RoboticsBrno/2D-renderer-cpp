#include "Shapes/Collection.hpp"

Collection::Collection(const ShapeParams &params) : Shape(params) {}

Collection::~Collection() { clear(); }

Collider *Collection::defaultCollider() { return new CircleCollider(0, 0, 0); }

void Collection::addShape(std::shared_ptr<Shape> shape) {
    if (shape) {
        shape->setParent(this);
        shapes.push_back(shape);
        this->isDirty = true;
    }
}

void Collection::removeShape(std::shared_ptr<Shape> shape) {
    auto it = std::remove(shapes.begin(), shapes.end(), shape);
    if (it != shapes.end()) {
        (*it)->setParent(nullptr);
        shapes.erase(it, shapes.end());
        this->isDirty = true;
    }
}

void Collection::clear() {
    for (auto &shape : shapes) {
        if (shape)
            shape->setParent(nullptr);
    }
    shapes.clear();
    cachedSortedShapes.clear();
    this->isDirty = true;
}

void Collection::drawAliased(Pixels &pixels) {
    if (this->isDirty) {
        this->cachedSortedShapes = shapes;
        std::sort(this->cachedSortedShapes.begin(),
                  this->cachedSortedShapes.end(),
                  [](const std::shared_ptr<Shape> &a,
                     const std::shared_ptr<Shape> &b) {
                      return a->getZ() < b->getZ();
                  });
        this->isDirty = false;
    }

    for (const auto &shape : this->cachedSortedShapes) {
        if (shape)
            shape->drawAliased(pixels);
    }
}

void Collection::drawAntiAliased(Pixels &pixels) {
    if (this->isDirty) {
        this->cachedSortedShapes = shapes;
        std::sort(this->cachedSortedShapes.begin(),
                  this->cachedSortedShapes.end(),
                  [](const std::shared_ptr<Shape> &a,
                     const std::shared_ptr<Shape> &b) {
                      return a->getZ() < b->getZ();
                  });
        this->isDirty = false;
    }

    for (const auto &shape : this->cachedSortedShapes) {
        if (shape)
            shape->drawAntiAliased(pixels);
    }
}
