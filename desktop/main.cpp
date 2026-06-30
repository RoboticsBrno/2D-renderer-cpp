#include "Renderer.hpp"
#include "Shapes/Circle.hpp"
#include "Shapes/Collection.hpp"
#include "Shapes/Polygon.hpp"
#include "Shapes/Rectangle.hpp"
#include "Shapes/RegularPolygon.hpp"
#include "Utils.hpp"
#include <cstdio>
#include <memory>

int main() {
    const int W = 64, H = 64;

    Renderer renderer(W, H);

    auto scene = std::make_shared<Collection>(ShapeParams{0, 0, Colors::BLACK, 0});

    scene->addShape(std::make_shared<Rectangle>(
        RectangleParams{4, 4, Colors::RED, 20, 20, true}));

    scene->addShape(std::make_shared<Circle>(
        CircleParams{44, 44, Colors::BLUE, 14, true}));

    scene->addShape(std::make_shared<RegularPolygon>(
        RegularPolygonRadiusParams{32, 32, Colors::GREEN, 6, 12, false}));

    scene->addShape(std::make_shared<Polygon>(PolygonParams{
        10, 40, Colors::YELLOW,
        {{0, 0}, {12, 0}, {16, 10}, {6, 16}, {-4, 10}}, true}));

    DrawOptions opts{W, H, true};
    renderer.render({scene}, opts);

    const Display &d = renderer.getDisplayGrid();
    int nonBlack = 0;
    for (const auto &p : d.pixels)
        if (p.r || p.g || p.b)
            nonBlack++;

    printf("Rendered %dx%d grid: %d non-black pixels\n", W, H, nonBlack);
    return nonBlack > 0 ? 0 : 1;
}
