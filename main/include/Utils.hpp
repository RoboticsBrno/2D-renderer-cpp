#pragma once
#include <cstdint>
#include <vector>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    float a;

    Color() : r(0), g(0), b(0), a(1.0f) {}
    Color(uint8_t r, uint8_t g, uint8_t b, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
};

struct Pixel {
    int x;
    int y;
    Color color;

    Pixel() : x(0), y(0), color() {}
    Pixel(int x, int y, Color color) : x(x), y(y), color(color) {}
};

using Pixels = std::vector<Pixel>;

class Texture;

// Note: loadTexture is omitted as it requires file system access
// You'll need to implement this based on your texture loading needs
