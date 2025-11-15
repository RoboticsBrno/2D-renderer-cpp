#include "Renderer.hpp"
#include <map>
#include <string>

Renderer::Renderer(int width, int height, const Color &backgroundColor)
    : width(width), height(height), backgroundColor(backgroundColor) {}

Pixels Renderer::render(const std::vector<Collection *> &collections,
                        const DrawOptions &options) {
    Pixels pixels;

    // Sort collections by z-index
    std::vector<Collection *> sortedCollections = collections;
    std::sort(
        sortedCollections.begin(), sortedCollections.end(),
        [](Collection *a, Collection *b) { return a->getZ() < b->getZ(); });

    for (Collection *collection : sortedCollections) {
        Pixels collectionPixels = collection->draw(options);
        pixels.insert(pixels.end(), collectionPixels.begin(),
                      collectionPixels.end());
    }

    return pixels;
}

Pixels Renderer::blendPixels(const Pixels &pixels) {
    std::map<std::string, Pixel> pixelMap;

    for (const Pixel &pixel : pixels) {
        std::string key =
            std::to_string(pixel.x) + "," + std::to_string(pixel.y);

        if (pixelMap.find(key) != pixelMap.end()) {
            const Pixel &existingPixel = pixelMap[key];
            float threshold = 0.999f;
            if (pixel.color.a >= threshold) {
                pixelMap[key] = pixel;
            } else if (existingPixel.color.a < threshold) {
                Pixel blended = blendPixel(existingPixel, pixel);
                pixelMap[key] = blended;
            }
        } else {
            pixelMap[key] = pixel;
        }
    }

    Pixels result;
    for (const auto &pair : pixelMap) {
        result.push_back(pair.second);
    }
    return result;
}

Pixel Renderer::blendPixel(const Pixel &background, const Pixel &foreground) {
    float alpha = foreground.color.a;

    return Pixel(foreground.x, foreground.y,
                 Color(static_cast<uint8_t>(foreground.color.r * alpha +
                                            background.color.r * (1 - alpha)),
                       static_cast<uint8_t>(foreground.color.g * alpha +
                                            background.color.g * (1 - alpha)),
                       static_cast<uint8_t>(foreground.color.b * alpha +
                                            background.color.b * (1 - alpha)),
                       std::min(1.0f, background.color.a +
                                          alpha * (1 - background.color.a))));
}
