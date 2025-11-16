#include "Renderer.hpp"
#include "Shapes/Circle.hpp"
#include "Shapes/Collection.hpp"
#include "Shapes/Rectangle.hpp"
#include "Utils.hpp"
#include "esp_timer.h"
#include "freertos/projdefs.h"
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

    Collection *earthCollection =
        new Collection(ShapeParams{32, 32, Color(0, 0, 0, 1.0f), 0});
    earthCollection->setPivot(32.0f, 32.0f);

    Circle *earth = new Circle(
        CircleParams{32 + 20, 32, Color(0, 100, 255, 1.0f), 4, true});
    earthCollection->addShape(earth);

    Circle *alien = new Circle(
        CircleParams{32 - 20, 32, Color(0, 255, 100, 1.0f), 4, true});
    earthCollection->addShape(alien);

    Collection *moonCollection =
        new Collection(ShapeParams{32 + 20, 32, Color(0, 0, 0, 1.0f), 0});
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
    while (true) {
        earthCollection->rotate(1.5f);
        moonCollection->rotate(3.0f);
        uint64_t startTime = esp_timer_get_time();
        Pixels pixels = renderer.render(collections, options);
        uint64_t endTime = esp_timer_get_time();
        sumTime += (endTime - startTime);
        counter++;
        if (counter % 30 == 0) {
            float avgTimeMs = (sumTime / counter) / 1000.0f;
            printf("Average render time over %d frames: %.2f ms\n", counter,
                   avgTimeMs);
            counter = 0;
            sumTime = 0;
        }
        display.setBuffer(pixels);
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    // Cleanup
    for (auto collection : collections) {
        delete collection;
    }
}

extern "C" void app_main(void) {
    printf("C++/TypeScript Output Comparison\n");
    printf("================================\n");

    runSolarSystem();
}
