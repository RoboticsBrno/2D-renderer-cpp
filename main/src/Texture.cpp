#include "Texture.hpp"
#include "Profiler.hpp"
#include "esp_littlefs.h"
#include "esp_log.h"
#include <algorithm>
#include <cmath>
#include <cstring>

static const char *TAG = "Texture";

bool Texture::initFS() {

    esp_vfs_littlefs_conf_t conf = {.base_path = "/lfs",
                                    .partition_label = "lfs",
                                    .partition = NULL,
                                    .format_if_mount_failed = false,
                                    .read_only = false,
                                    .dont_mount = false,
                                    .grow_on_mount = false};

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount or format filesystem");
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get filesystem info");
        return false;
    }

    ESP_LOGI(TAG, "LittleFS mounted: total: %zu bytes, used: %zu bytes", total,
             used);

    return true;
}

Texture::Texture(const std::vector<std::vector<Color>> &pixels)
    : pixels(pixels), wrapMode("repeat"), valid(true) {
    this->height = pixels.size();
    this->width = (height > 0) ? pixels[0].size() : 0;
}

Texture::Texture() : width(0), height(0), wrapMode("repeat"), valid(false) {}

bool Texture::readFile(const std::string &filename,
                       std::vector<uint8_t> &buffer) {
    std::string fullPath = "/lfs/" + filename;

    FILE *file = fopen(fullPath.c_str(), "rb");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", fullPath.c_str());
        return false;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0) {
        ESP_LOGE(TAG, "Invalid file size: %ld", fileSize);
        fclose(file);
        return false;
    }

    // Read file content
    buffer.resize(fileSize);
    size_t bytesRead = fread(buffer.data(), 1, fileSize, file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize)) {
        ESP_LOGE(TAG, "Failed to read file: read %zu of %ld bytes", bytesRead,
                 fileSize);
        return false;
    }

    ESP_LOGI(TAG, "Successfully read %s, size: %zu bytes", filename.c_str(),
             bytesRead);
    return true;
}
uint16_t Texture::getUint16(const uint8_t *data, size_t offset,
                            bool littleEndian) {
    if (littleEndian) {
        return data[offset] | (data[offset + 1] << 8);
    } else {
        return (data[offset] << 8) | data[offset + 1];
    }
}

uint32_t Texture::getUint32(const uint8_t *data, size_t offset,
                            bool littleEndian) {
    if (littleEndian) {
        return data[offset] | (data[offset + 1] << 8) |
               (data[offset + 2] << 16) | (data[offset + 3] << 24);
    } else {
        return (data[offset] << 24) | (data[offset + 1] << 16) |
               (data[offset + 2] << 8) | data[offset + 3];
    }
}

int32_t Texture::getInt32(const uint8_t *data, size_t offset,
                          bool littleEndian) {
    return static_cast<int32_t>(getUint32(data, offset, littleEndian));
}

bool Texture::fromBMP(const std::string &filename, Texture &outTexture,
                      bool littleEndian) {
    std::vector<uint8_t> fileData;

    if (!readFile(filename, fileData)) {
        ESP_LOGE(TAG, "Failed to read BMP file: %s", filename.c_str());
        return false;
    }

    const uint8_t *data = fileData.data();

    // Check BMP signature
    uint16_t signature = getUint16(data, 0, littleEndian);
    if (signature != 0x4D42) {
        ESP_LOGE(TAG, "Invalid BMP file signature: 0x%04X", signature);
        return false;
    }

    uint32_t pixelDataOffset = getUint32(data, 10, littleEndian);
    uint32_t headerSize = getUint32(data, 14, littleEndian); // Add this!
    int32_t width = getInt32(data, 18, littleEndian);
    int32_t height = getInt32(data, 22, littleEndian);
    uint16_t bitsPerPixel = getUint16(data, 28, littleEndian);

    ESP_LOGI(TAG,
             "BMP info: %dx%d, %d bpp, header: %d bytes, data offset: 0x%X",
             width, height, bitsPerPixel, headerSize, pixelDataOffset);

    // Handle different header sizes
    if (headerSize != 40 && headerSize != 124 && headerSize != 108 &&
        headerSize != 56) {
        ESP_LOGW(TAG, "Unusual BMP header size: %d bytes", headerSize);
    }

    if (width <= 0 || height <= 0) {
        ESP_LOGE(TAG, "Invalid BMP dimensions: %dx%d", width, height);
        return false;
    }

    if (pixelDataOffset >= fileData.size()) {
        ESP_LOGE(TAG, "Pixel data offset out of bounds");
        return false;
    }

    std::vector<std::vector<Color>> pixels;

    if (bitsPerPixel == 24) {
        // 24-bit BMP (RGB)
        int bytesPerRow =
            ((width * 3 + 3) / 4) * 4; // Row size is padded to multiple of 4

        pixels.resize(height);
        for (int y = 0; y < height; y++) {
            pixels[y].resize(width);
            for (int x = 0; x < width; x++) {
                size_t offset =
                    pixelDataOffset + (height - 1 - y) * bytesPerRow + x * 3;
                if (offset + 2 >= fileData.size()) {
                    ESP_LOGE(TAG, "BMP data out of bounds at offset %zu",
                             offset);
                    return false;
                }

                uint8_t b = data[offset];
                uint8_t g = data[offset + 1];
                uint8_t r = data[offset + 2];
                pixels[y][x] = Color(r, g, b, 1.0f);
            }
        }
    } else if (bitsPerPixel == 32) {
        // 32-bit BMP (RGBA)
        pixels.resize(height);
        for (int y = 0; y < height; y++) {
            pixels[y].resize(width);
            for (int x = 0; x < width; x++) {
                size_t offset =
                    pixelDataOffset + (height - 1 - y) * width * 4 + x * 4;
                if (offset + 3 >= fileData.size()) {
                    ESP_LOGE(TAG, "BMP data out of bounds at offset %zu",
                             offset);
                    return false;
                }

                uint8_t b = data[offset];
                uint8_t g = data[offset + 1];
                uint8_t r = data[offset + 2];
                uint8_t a = data[offset + 3];
                pixels[y][x] = Color(r, g, b, a / 255.0f);
            }
        }
    } else {
        ESP_LOGE(TAG, "Unsupported BMP format: %d bits per pixel",
                 bitsPerPixel);
        return false;
    }

    outTexture = Texture(pixels);
    ESP_LOGI(TAG, "Successfully loaded BMP texture: %dx%d, %d bpp", width,
             height, bitsPerPixel);
    return true;
}
Color Texture::sample(int u, int v) const {
    if (!valid || pixels.empty() || width == 0 || height == 0) {
        return Color(0, 0, 0, 1.0f);
    }

    int x = u;
    int y = v;

    if (wrapMode == "repeat") {
        if (u >= 0 && u < width && v >= 0 && v < height) {
            return pixels[y][x];
        } else {
            x = u % width;
            y = v % height;
            if (x < 0)
                x += width;
            if (y < 0)
                y += height;
        }
    } else {
        x = std::max(0, std::min(width - 1, x));
        y = std::max(0, std::min(height - 1, y));
    }

    if (y >= 0 && y < height && x >= 0 && x < width) {
        return pixels[y][x];
    }

    return Color(0, 0, 0, 1.0f);
}

void Texture::setWrapMode(const std::string &mode) {
    if (mode == "repeat" || mode == "clamp") {
        wrapMode = mode;
    } else {
        ESP_LOGW(TAG, "Invalid wrap mode: %s, using 'repeat'", mode.c_str());
        wrapMode = "repeat";
    }
}
