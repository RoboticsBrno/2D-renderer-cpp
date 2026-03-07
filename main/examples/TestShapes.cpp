#include "examples/TestShapes.hpp"
#include "LineSegment.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "RegularPolygon.hpp"
#include "Renderer.hpp"
#include "Shapes/Circle.hpp"
#include "Shapes/Collection.hpp"
#include "Shapes/Rectangle.hpp"
#include "Utils.hpp"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include <cstdio>
#include <memory>
#include <stdio.h>

void runTestShapes() {
    const int width = 64;
    const int height = 64;

    Renderer renderer(width, height, Color(0, 0, 0, 1.0f));

    auto mainCollection = std::make_shared<Collection>(
        ShapeParams{32, 32, Color(0, 0, 0, 1.0f), 0});

    auto rect = std::make_shared<Rectangle>(
        RectangleParams{6, 6, Color(255, 0, 0, 1.0f), 6, 6, true});

    auto circle = std::make_shared<Circle>(
        CircleParams{18, 6, Color(0, 0, 255, 1.0f), 5, true});

    auto hexagon = std::make_shared<RegularPolygon>(RegularPolygonRadiusParams{
        48, 48, Color(0, 255, 0, 1.0f), 6, 10, true});

    auto pentagon = std::make_shared<RegularPolygon>(RegularPolygonRadiusParams{
        48, 16, Color(255, 255, 0, 1.0f), 5, 8, true});

    auto polygon = std::make_shared<Polygon>(PolygonParams{
        16, 48, Color(255, 0, 255, 1.0f),
        std::vector<std::pair<int, int>>{{0, 0}, {10, 5}, {5, 15}, {0, 10}},
        true});

    auto line = std::make_shared<LineSegment>(
        LineSegmentParams{0, 20, Color(255, 255, 255, 1.0f), 63, 35, 0});

    auto line2 = std::make_shared<LineSegment>(
        LineSegmentParams{32, 0, Color(0, 100, 50, .5f), 20, 40, 0});

    auto lineRect = std::make_shared<Rectangle>(
        RectangleParams{25, 20, Color(155, 0, 0, 0.9f), 8, 5, true});

    auto point = std::make_shared<Point>(
        ShapeParams{32, 42, Color(155, 255, 255, 1.0f), 0});

    mainCollection->addShape(rect);
    mainCollection->addShape(circle);
    mainCollection->addShape(hexagon);
    mainCollection->addShape(pentagon);
    mainCollection->addShape(polygon);
    mainCollection->addShape(line);
    mainCollection->addShape(line2);
    mainCollection->addShape(lineRect);
    mainCollection->addShape(point);

    DrawOptions options = {width, height, true}; // Anti-aliased
    HUB75Display display(width, height);
    if (!display.isInitialized()) {
        printf("Display not initialized. Skipping display output.\n");
        return;
    }

    Pixels pixels;
    renderer.render(pixels,
                    std::vector<std::shared_ptr<Collection>>{mainCollection},
                    options);
    while (true) {
        display.setBuffer(pixels);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
