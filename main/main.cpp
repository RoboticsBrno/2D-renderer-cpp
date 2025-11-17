#include "LineSegment.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "Profiler.hpp"
#include "RegularPolygon.hpp"
#include "Renderer.hpp"
#include "Shapes/Circle.hpp"
#include "Shapes/Collection.hpp"
#include "Shapes/Rectangle.hpp"
#include "Utils.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"
#include <cstdint>
#include <cstdio>
#include <stdio.h>
#include <vector>
void runSolarSystem() {
    const int width = 64;
    const int height = 64;
    Renderer renderer(width, height, Color(0, 0, 0, 1.0f));

    std::vector<Collection *> collections;

    Collection *sunCollection =
        new Collection(ShapeParams{32, 32, Color(0, 0, 0, 1.0f), 0});
    sunCollection->setPivot(32.0f, 32.0f);
    collections.push_back(sunCollection);

    Circle *sun =
        new Circle(CircleParams{32, 32, Color(255, 204, 0, 1.0f), 8, true});
    sunCollection->addShape(sun);

    std::vector<std::pair<int, int>> polygonVertices = {
        {60, 60}, {63, 61}, {61, 63}};
    Polygon *polygon = new Polygon(
        PolygonParams{32, 32, Color(255, 150, 0, 1.0f), polygonVertices, true});
    sunCollection->addShape(polygon);

    Collection *earthCollection =
        new Collection(ShapeParams{32, 32, Color(0, 0, 0, 1.0f), 1});
    earthCollection->setPivot(32.0f, 32.0f);

    Circle *earth = new Circle(
        CircleParams{32 + 20, 32, Color(0, 100, 255, 1.0f), 4, true});
    earthCollection->addShape(earth);

    Circle *alien = new Circle(
        CircleParams{32 - 20, 32, Color(0, 255, 100, 1.0f), 4, true});
    earthCollection->addShape(alien);

    Collection *moonCollection =
        new Collection(ShapeParams{32 + 20, 32, Color(0, 0, 0, 1.0f), 1});
    Circle *moon = new Circle(
        CircleParams{32 + 20 + 8, 32, Color(200, 200, 200, 1.0f), 2, true});
    moonCollection->addShape(moon);

    earthCollection->addShape(moonCollection);
    sunCollection->addShape(earthCollection);

    Circle *earthOrbit =
        new Circle(CircleParams{32, 32, Color(100, 100, 100, 0.3f), 20, false});
    Circle *moonOrbit = new Circle(
        CircleParams{32 + 20, 32, Color(100, 100, 100, 0.3f), 8, false});
    sunCollection->addShape(earthOrbit);
    earthCollection->addShape(moonOrbit);

    DrawOptions options = {width, height, true}; // Anti-aliased

    HUB75Display display(width, height);
    if (!display.isInitialized()) {
        printf("Display not initialized. Skipping display output.\n");
        return;
    }

    display.clear();
    int counter = 0;
    uint64_t sumTime = 0;
    uint64_t renderTime = 0;

    const float TARGET_FPS = 200.0f;
    const uint64_t TARGET_FRAME_TIME_US = 1000000 / TARGET_FPS;
    while (true) {
        uint64_t frameStartTime = esp_timer_get_time();

        earthCollection->rotate(1.5f);
        moonCollection->rotate(3.0f);

        Pixels pixels;
        renderer.render(pixels, collections, options);
        display.setBuffer(pixels);

        uint64_t frameEndTime = esp_timer_get_time();
        uint64_t frameTime = frameEndTime - frameStartTime;

        vTaskDelay(1);
        if (frameTime < TARGET_FRAME_TIME_US) {
            uint64_t remainingTime = TARGET_FRAME_TIME_US - frameTime;
            TickType_t delayTicks =
                ((remainingTime / 1000) / portTICK_PERIOD_MS);
            if (delayTicks > 0) {
                vTaskDelay(delayTicks);
            }
        }

        uint64_t adjustedFrameEndTime = esp_timer_get_time();
        uint64_t adjustedFrameTime = adjustedFrameEndTime - frameStartTime;
        sumTime += adjustedFrameTime;
        renderTime += frameTime;
        counter++;
        if (counter % 30 == 0) {
            float avgFrameTimeMs = (sumTime / counter) / 1000.0f;
            float currentFps = 1000000.0f / (avgFrameTimeMs * 1000.0f);

            printf("%.2f ms/frame (%.1f FPS) over %d frames\n", avgFrameTimeMs,
                   currentFps, counter);
            printf("  Render time: %.2f ms/frame\n",
                   (renderTime / counter) / 1000.0f);

            counter = 0;
            sumTime = 0;
            renderTime = 0;
        }
    }
    // Cleanup
    for (auto collection : collections) {
        delete collection;
    }
}

void runTestShapes() {
    const int width = 64;
    const int height = 64;

    Renderer renderer(width, height, Color(0, 0, 0, 1.0f));

    Collection *mainCollection =
        new Collection(ShapeParams{32, 32, Color(0, 0, 0, 1.0f), 0});

    Rectangle *rect = new Rectangle(
        RectangleParams{6, 6, Color(255, 0, 0, 1.0f), 6, 6, true});

    Circle *circle =
        new Circle(CircleParams{18, 6, Color(0, 0, 255, 1.0f), 5, true});

    RegularPolygon *hexagon = new RegularPolygon(RegularPolygonRadiusParams{
        48, 48, Color(0, 255, 0, 1.0f), 6, 10, true});
    RegularPolygon *pentagon = new RegularPolygon(RegularPolygonRadiusParams{
        48, 16, Color(255, 255, 0, 1.0f), 5, 8, true});

    Polygon *polygon = new Polygon(PolygonParams{
        16, 48, Color(255, 0, 255, 1.0f),
        std::vector<std::pair<int, int>>{{0, 0}, {10, 5}, {5, 15}, {0, 10}},
        true});

    LineSegment *line = new LineSegment(
        LineSegmentParams{0, 20, Color(255, 255, 255, 1.0f), 63, 35, 0});

    LineSegment *line2 = new LineSegment(
        LineSegmentParams{32, 0, Color(0, 100, 50, .5f), 20, 40, 0});

    Rectangle *lineRect = new Rectangle(
        RectangleParams{25, 20, Color(155, 0, 0, 0.9f), 8, 5, true});

    Point *point =
        new Point(ShapeParams{32, 42, Color(155, 255, 255, 1.0f), 0});

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
    PROFILE_FUNC(renderer.render(
        pixels, std::vector<Collection *>{mainCollection}, options));
    profile_print_results();
    while (true) {
        display.setBuffer(pixels);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Cleanup
    delete mainCollection;
}

void runCollisionTest() {
    const int width = 64;
    const int height = 64;
    Renderer renderer(width, height, Color(0, 0, 0, 1.0f));
    Collection *mainCollection =
        new Collection(ShapeParams{32, 32, Color(0, 0, 0, 1.0f), 0});

    Rectangle *ground = new Rectangle(
        RectangleParams{0, 50, Color(100, 50, 0, 1.0f), 64, 7, true});
    ground->addCollider();
    RegularPolygon *ball = new RegularPolygon(RegularPolygonRadiusParams(
        32, 10, Color(255, 255, 0, 1.0f), 6, 8, true));
    ball->addCollider();
    mainCollection->addShape(ground);
    mainCollection->addShape(ball);

    DrawOptions options = {width, height, true}; // Anti-aliased
    HUB75Display display(width, height);
    if (!display.isInitialized()) {
        printf("Display not initialized. Skipping display output.\n");
        return;
    }

    float velocityY = 0.0f;
    float velocityX = 1.0f;
    const float gravity = 0.5f;

    while (true) {
        // Handle collisions first (using velocity from previous frame)
        if (ball->intersects(ground)) {
            ball->setPosition(ball->getX(), ground->getY() - ball->getRadius());
            velocityY -= gravity;
            velocityY = -velocityY; // Perfect bounce
        }

        if (ball->getX() - ball->getRadius() <= 0 ||
            ball->getX() + ball->getRadius() >= width) {
            velocityX = -velocityX;
        }

        // Apply movement
        ball->translate(velocityX, velocityY);

        // Apply gravity for next frame
        velocityY += gravity;

        Pixels pixels;
        renderer.render(pixels, std::vector<Collection *>{mainCollection},
                        options);
        display.setBuffer(pixels);
        vTaskDelay(1);
    }
}

extern "C" int app_main(void) {
    runTestShapes();
    return 0;
}
