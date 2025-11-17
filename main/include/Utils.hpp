#pragma once
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
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

namespace Colors {
static const Color BLACK(0, 0, 0);
static const Color WHITE(255, 255, 255);
static const Color RED(255, 0, 0);
static const Color GREEN(0, 255, 0);
static const Color BLUE(0, 0, 255);
static const Color YELLOW(255, 255, 0);
static const Color MAGENTA(255, 0, 255);
static const Color CYAN(0, 255, 255);
} // namespace Colors

class HUB75Display {
  private:
    MatrixPanel_I2S_DMA *display;
    int width;
    int height;
    bool initialized;
    static const char *TAG;
    Pixels previousBuffer;

    HUB75_I2S_CFG::i2s_pins getDefaultPins() {
        return {
            .r1 = 4,   // R1 - Red for upper half
            .g1 = 5,   // G1 - Green for upper half
            .b1 = 6,   // B1 - Blue for upper half
            .r2 = 7,   // R2 - Red for lower half
            .g2 = 15,  // G2 - Green for lower half
            .b2 = 16,  // B2 - Blue for lower half
            .a = 18,   // A - Address line
            .b = 8,    // B - Address line
            .c = 3,    // C - Address line
            .d = 42,   // D - Address line
            .e = 17,   // E - Address line (set to -1 for 32x32 panels)
            .lat = 40, // LAT - Latch pin
            .oe = 2,   // OE - Output Enable pin
            .clk = 41  // CLK - Clock pin
        };
    }

  public:
    HUB75Display(int panelWidth = 64, int panelHeight = 32,
                 int chainLength = 1);

    ~HUB75Display();

    void setPixel(int x, int y, const Color &color);

    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

    void setPixels(const Pixels &pixels);

    void setBuffer(const Pixels &pixels, bool clearPrevious = true);

    void fill(const Color &color);

    void clear();

    void setBrightness(uint8_t brightness);

    bool isInitialized() const;

    bool isValidCoordinate(int x, int y) const;
};
