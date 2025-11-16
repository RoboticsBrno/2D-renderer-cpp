#include "Utils.hpp"
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"

HUB75Display::HUB75Display(int panelWidth, int panelHeight, int chainLength)
    : display(nullptr), width(panelWidth * chainLength), height(panelHeight),
      initialized(false) {

    ESP_LOGI(TAG, "Creating HUB75 display: %dx%d (chain: %d)", width, height,
             chainLength);

    HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, chainLength,
                           getDefaultPins(), HUB75_I2S_CFG::SHIFTREG,
                           HUB75_I2S_CFG::TYPE138,
                           false, // double buffer
                           HUB75_I2S_CFG::HZ_10M, DEFAULT_LAT_BLANKING,
                           true, // clock phase
                           120,  // min refresh rate
                           8     // pixel color depth
    );

    // Create display instance
    display = new MatrixPanel_I2S_DMA(mxconfig);

    // Initialize display
    if (display->begin()) {
        initialized = true;
        display->setBrightness8(90);
        display->clearScreen();
        ESP_LOGI(TAG, "Display initialized successfully");
    } else {
        ESP_LOGE(TAG, "Failed to initialize display");
    }
}

HUB75Display::~HUB75Display() {
    if (display) {
        delete display;
    }
}

// Set a pixel with Color struct (supports alpha blending)
void HUB75Display::setPixel(int x, int y, const Color &color) {
    if (!initialized || !isValidCoordinate(x, y)) {
        return;
    }

    display->drawPixelRGB888(x, y, color.r * color.a, color.g * color.a,
                             color.b * color.a);
}

// Set a pixel with RGB values (convenience method)
void HUB75Display::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    setPixel(x, y, Color(r, g, b));
}

void HUB75Display::setPixels(const Pixels &pixels) {
    if (!initialized)
        return;

    for (const auto &pixel : pixels) {
        setPixel(pixel.x, pixel.y, pixel.color);
    }
}

void HUB75Display::setBuffer(const Pixels &pixels) {
    if (!initialized)
        return;
    display->clearScreen();
    for (const auto &pixel : pixels) {
        setPixel(pixel.x, pixel.y, pixel.color);
    }
}

// Fill the entire display with a color
void HUB75Display::fill(const Color &color) {
    if (!initialized)
        return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            setPixel(x, y, color);
        }
    }
}

// Clear the display (fill with black)
void HUB75Display::clear() {
    if (initialized) {
        display->clearScreen();
    }
}

bool HUB75Display::isInitialized() const { return initialized; }

// Set display brightness (0-255)
void HUB75Display::setBrightness(uint8_t brightness) {
    if (initialized) {
        display->setBrightness8(brightness);
    }
}

bool HUB75Display::isValidCoordinate(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

const char *HUB75Display::TAG = "HUB75Display";
