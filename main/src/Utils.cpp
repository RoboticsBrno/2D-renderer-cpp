#include "Utils.hpp"
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include <cstring>

bool operator==(const Color &lhs, const Color &rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

bool operator==(const Pixel &lhs, const Pixel &rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.color == rhs.color;
}

bool operator!=(const Pixel &lhs, const Pixel &rhs) { return !(lhs == rhs); }

HUB75Display::HUB75Display(int panelWidth, int panelHeight, int chainLength)
    : display(nullptr), width(panelWidth * chainLength), height(panelHeight),
      initialized(false) {

    ESP_LOGI(TAG, "Creating HUB75 display: %dx%d (chain: %d)", width, height,
             chainLength);

    HUB75_I2S_CFG mxconfig(panelWidth, panelHeight, chainLength,
                           getDefaultPins(), HUB75_I2S_CFG::SHIFTREG,
                           HUB75_I2S_CFG::TYPE138, false, HUB75_I2S_CFG::HZ_10M,
                           DEFAULT_LAT_BLANKING, true, 120, 8);

    display = new MatrixPanel_I2S_DMA(mxconfig);

    if (display->begin()) {
        initialized = true;

        previousBuffer.resize(width * height, Pixel{0, 0, Colors::BLACK});
        m_current_frame_buffer.resize(width * height);
        m_pixel_is_set.resize(width * height);

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

void HUB75Display::setPixel(int x, int y, const Color &color) {
    if (!initialized || !isValidCoordinate(x, y)) {
        return;
    }

    display->drawPixelRGB888(x, y, color.r * color.a, color.g * color.a,
                             color.b * color.a);
}

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

void HUB75Display::setBuffer(const Pixels &pixels, bool clearPrevious) {
    if (!initialized)
        return;

    memset(m_pixel_is_set.data(), 0, m_pixel_is_set.size());

    for (const auto &pixel : pixels) {
        if (isValidCoordinate(pixel.x, pixel.y)) {
            int index = pixel.y * width + pixel.x;
            m_current_frame_buffer[index] = pixel.color;
            m_pixel_is_set[index] = 1;
        }
    }

    int i = 0;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x, ++i) {
            const Color &old_color = previousBuffer[i].color;
            Color new_color;

            if (m_pixel_is_set[i]) {
                new_color = m_current_frame_buffer[i];
            } else {
                if (clearPrevious) {
                    new_color = Colors::BLACK;
                } else {
                    new_color = old_color;
                }
            }

            if (!(new_color == old_color)) {
                setPixel(x, y, new_color);
                previousBuffer[i].color = new_color;
                previousBuffer[i].x = x;
                previousBuffer[i].y = y;
            }
        }
    }
}
void HUB75Display::fill(const Color &color) {
    if (!initialized)
        return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            setPixel(x, y, color);
        }
    }
}

void HUB75Display::clear() {
    if (initialized) {
        display->clearScreen();
    }
}

bool HUB75Display::isInitialized() const { return initialized; }

void HUB75Display::setBrightness(uint8_t brightness) {
    if (initialized) {
        display->setBrightness8(brightness);
    }
}

bool HUB75Display::isValidCoordinate(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

const char *HUB75Display::TAG = "HUB75Display";
