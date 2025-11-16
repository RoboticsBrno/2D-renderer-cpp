#include "Renderer.hpp"
#include "Profiler.hpp"

Renderer::Renderer(int width, int height, const Color &backgroundColor)
    : width(width), height(height), backgroundColor(backgroundColor) {}

Pixels Renderer::render(const std::vector<Collection *> &collections,
                        const DrawOptions &options) {
    Pixels pixels;

    std::vector<Collection *> sortedCollections = collections;
    std::sort(
        sortedCollections.begin(), sortedCollections.end(),
        [](Collection *a, Collection *b) { return a->getZ() < b->getZ(); });

    for (Collection *collection : sortedCollections) {
        Pixels collectionPixels = PROFILE_FUNC_RET(collection->draw(options));
        pixels.insert(pixels.end(), collectionPixels.begin(),
                      collectionPixels.end());
    }

    return pixels;
}
