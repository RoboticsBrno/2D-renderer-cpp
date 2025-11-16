#include "Collection.hpp"
#include "../Profiler.hpp"

template <typename T, typename Compare>
void simple_sort(std::vector<T> &vec, Compare comp) {
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = i + 1; j < vec.size(); ++j) {
            if (comp(vec[j], vec[i])) {
                std::swap(vec[i], vec[j]);
            }
        }
    }
}

Collection::Collection(const ShapeParams &params) : Shape(params) {}

Collection::~Collection() {
    for (auto shape : shapes) {
        delete shape;
    }
}

Collider *Collection::defaultCollider() { return new CircleCollider(0, 0, 0); }

void Collection::addShape(Shape *shape) {
    shape->setParent(this);
    shapes.push_back(shape);
}

Pixels Collection::drawAntiAliased() {
    Pixels pixels;

    std::vector<Shape *> sortedShapes = shapes;
    simple_sort(sortedShapes,
                [](Shape *a, Shape *b) { return a->getZ() < b->getZ(); });

    for (Shape *shape : sortedShapes) {
        Pixels shapePixels = PROFILE_FUNC_RET(shape->drawAntiAliased());
        pixels.insert(pixels.end(), shapePixels.begin(), shapePixels.end());
    }

    return pixels;
}

Pixels Collection::drawAliased() {
    Pixels pixels;

    std::vector<Shape *> sortedShapes = shapes;
    simple_sort(sortedShapes,
                [](Shape *a, Shape *b) { return a->getZ() < b->getZ(); });

    for (Shape *shape : sortedShapes) {
        Pixels shapePixels = shape->drawAliased();
        pixels.insert(pixels.end(), shapePixels.begin(), shapePixels.end());
    }
    return pixels;
}
