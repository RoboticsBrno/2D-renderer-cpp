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
    Display displayGrid;

  public:
    Renderer(int width, int height);

    void render(const std::vector<std::shared_ptr<Collection>> &collections,
                const DrawOptions &options);

    void drawText(const std::string &text, int x, int y, const Font &font,
                  const Color &color, bool wrap = false);

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    const Display &getDisplayGrid() const { return displayGrid; }
    void clear();
};
