#pragma once
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <cstdlib>
#include <vector>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color() : r(0), g(0), b(0), a(255) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
};

bool operator==(const Color &lhs, const Color &rhs);
bool operator!=(const Color &lhs, const Color &rhs);

template <class T> struct PsramAllocator {
    typedef T value_type;

    PsramAllocator() = default;
    template <class U>
    constexpr PsramAllocator(const PsramAllocator<U> &) noexcept {}

    T *allocate(std::size_t n) {
        void *p = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM);
        if (!p) {
            ESP_LOGE("PSRAM",
                     "CRITICAL: Failed to allocate %zu bytes in PSRAM!",
                     n * sizeof(T));
            std::abort();
        }
        return static_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t) noexcept { heap_caps_free(p); }
};

struct Display {
    std::vector<Color, PsramAllocator<Color>> pixels;
    int width = 0;
    int height = 0;
};

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
