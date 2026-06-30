#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#ifdef ESP_PLATFORM
#include "esp_log.h"
#define RENDERER_LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#define RENDERER_LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define RENDERER_LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#else
#define RENDERER_LOGE(tag, fmt, ...) fprintf(stderr, "[E][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define RENDERER_LOGI(tag, fmt, ...) fprintf(stderr, "[I][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define RENDERER_LOGW(tag, fmt, ...) fprintf(stderr, "[W][%s] " fmt "\n", tag, ##__VA_ARGS__)
#endif

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"

template <class T> struct PsramAllocator {
    typedef T value_type;

    PsramAllocator() = default;
    template <class U>
    constexpr PsramAllocator(const PsramAllocator<U> &) noexcept {}

    T *allocate(std::size_t n) {
        void *p = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM);
        if (!p) {
            RENDERER_LOGE("PSRAM", "CRITICAL: Failed to allocate %zu bytes in PSRAM!",
                          n * sizeof(T));
            std::abort();
        }
        return static_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t) noexcept { heap_caps_free(p); }
};

#else
#include <memory>
template <class T> using PsramAllocator = std::allocator<T>;
#endif

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
}
