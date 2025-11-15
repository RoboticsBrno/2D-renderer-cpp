#include "Texture.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

Texture::Texture(const std::vector<std::vector<Color>> &pixels)
    : pixels(pixels), wrapMode("repeat") {
    height = pixels.size();
    width = height > 0 ? pixels[0].size() : 0;
}

Color Texture::sample(float u, float v) const {
    if (width == 0 || height == 0) {
        return Color(0, 0, 0, 1.0f);
    }

    float x = u;
    float y = v;

    if (std::strcmp(wrapMode, "repeat") == 0) {
        x = fmod(x, static_cast<float>(width));
        y = fmod(y, static_cast<float>(height));
        if (x < 0)
            x += width;
        if (y < 0)
            y += height;
    } else {
        x = std::max(0.0f, std::min(static_cast<float>(width - 1), x));
        y = std::max(0.0f, std::min(static_cast<float>(height - 1), y));
    }

    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));

    if (yi >= 0 && yi < height && xi >= 0 && xi < width) {
        return pixels[yi][xi];
    }

    return Color(0, 0, 0, 1.0f);
}

Texture Texture::fromBMP(const uint8_t *data, size_t length) {
    // BMP loading implementation would go here
    // For now, return a default texture
    std::vector<std::vector<Color>> defaultPixels = {{{255, 255, 255, 1.0f}}};
    return Texture(defaultPixels);
}
