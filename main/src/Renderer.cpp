#include "Renderer.hpp"
#include "Profiler.hpp"

Renderer::Renderer(int width, int height, const Color &backgroundColor)
    : width(width), height(height), backgroundColor(backgroundColor) {}

void Renderer::render(Pixels &pixels,
                        const std::vector<Collection *> &collections,
                        const DrawOptions &options) {
    pixels.reserve(10000);
    std::vector<Collection *> sortedCollections = collections;
    std::sort(
        sortedCollections.begin(), sortedCollections.end(),
        [](Collection *a, Collection *b) { return a->getZ() < b->getZ(); });

    for (Collection *collection : sortedCollections) {
        PROFILE_FUNC(collection->draw(pixels, options));
    }
}
