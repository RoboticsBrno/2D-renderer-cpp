#pragma once
#include "Utils.hpp"
#include <cstdint>
#include <vector>

class Texture {
  private:
    std::vector<std::vector<Color>> pixels;
    int width;
    int height;
    const char *wrapMode; // "repeat" or "clamp"

  public:
    Texture(const std::vector<std::vector<Color>> &pixels);

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    Color sample(int u, int v) const;
    void setWrapMode(const char *mode) { wrapMode = mode; }

    static Texture fromBMP(const uint8_t *data, size_t length);
};
