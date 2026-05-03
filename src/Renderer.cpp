#include "Renderer.hpp"
#include "Collection.hpp"
#include <algorithm>
#include <memory>
#include <vector>

Renderer::Renderer(int width, int height) : width(width), height(height) {
    displayGrid.width = width;
    displayGrid.height = height;
    displayGrid.pixels.resize(width * height, Color());
}

void Renderer::clear() {
    size_t bytesToClear =
        displayGrid.width * displayGrid.height * sizeof(Color);
    std::memset(displayGrid.pixels.data(), 0, bytesToClear);
}

void Renderer::render(
    const std::vector<std::shared_ptr<Collection>> &collections,
    const DrawOptions &options) {
    std::vector<std::shared_ptr<Collection>> sortedCollections = collections;
    std::sort(sortedCollections.begin(), sortedCollections.end(),
              [](std::shared_ptr<Collection> a, std::shared_ptr<Collection> b) {
                  return a->getZ() < b->getZ();
              });

    for (std::shared_ptr<Collection> collection : sortedCollections) {
        collection->draw(displayGrid, options);
    }
}

void Renderer::drawText(const std::string &text, int x, int y, const Font &font,
                        const Color &color, bool wrap) {
    int currentX = x;
    int currentY = y;
    int lineHeight = font.getHeight() + 1;

    auto setPixel = [&](int px, int py, const Color &c) {
        if (px >= 0 && px < width && py >= 0 && py < height) {
            displayGrid.pixels[py * width + px] = c;
        }
    };

    for (char c : text) {
        if (c == '\n') {
            currentX = x;
            currentY += lineHeight;
            continue;
        }

        const Font::Glyph *glyph = font.getGlyph(c);

        if (wrap && currentX > x) {
            int charWidth = glyph ? (glyph->width + font.getCharSpacing(c)) : 5;
            if (currentX + charWidth > width) {
                currentX = x;
                currentY += lineHeight;
            }
        }

        if (!glyph) {
            for (int i = 0; i < 5; ++i) {
                setPixel(currentX + i, currentY, color);
            }
            for (int i = 0; i < 5; ++i) {
                setPixel(currentX + i, currentY + 6, color);
            }
            for (int i = 0; i < 7; ++i) {
                setPixel(currentX, currentY + i, color);
            }
            for (int i = 0; i < 7; ++i) {
                setPixel(currentX + 4, currentY + i, color);
            }
            currentX += 5;
            continue;
        }

        for (int row = 0; row < glyph->height; ++row) {
            uint8_t line = glyph->data[row];

            for (int col = 0; col < glyph->width; ++col) {
                int bit_pos = 4 - (col + glyph->x_offset);
                if (bit_pos >= 0 && (line & (1 << bit_pos))) {
                    setPixel(currentX + col, currentY + row, color);
                }
            }
        }

        currentX += glyph->width + font.getCharSpacing(c);
    }
}
