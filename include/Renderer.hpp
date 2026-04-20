#pragma once
#include "Font/Font.hpp"
#include "Shapes/Collection.hpp"
#include "Utils.hpp"
#include <memory>
#include <string>
#include <vector>

class Renderer {
  private:
    int width;
    int height;
    Color backgroundColor;
    Display displayGrid;

  public:
    Renderer(int width, int height,
             const Color &backgroundColor = Color(0, 0, 0, 1.0f));

    void render(const std::vector<std::shared_ptr<Collection>> &collections,
                const DrawOptions &options);

    void drawText(const std::string &text, int x, int y, const Font &font,
                  const Color &color, bool wrap = false);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    const Display &getDisplayGrid() const { return displayGrid; }
    void clear();
};
