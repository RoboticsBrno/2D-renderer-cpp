#include "Renderer.hpp"
#include "Shapes/Circle.hpp"
#include "Shapes/Collection.hpp"
#include "Shapes/Rectangle.hpp"
#include <cstdio>
#include <stdio.h>
#include <vector>

// Function to generate a predictable test scene
void generateTestScene(std::vector<Collection *> &collections) {
    // Simple test scene with known positions and colors
    Collection *testCollection =
        new Collection(ShapeParams{0, 0, Color(0, 0, 0, 1.0f), 0});

    // Add a red circle at known position
    Circle *circle1 =
        new Circle(CircleParams{10, 10, Color(155, 0, 0, 1.0f), 10, true});
    testCollection->addShape(circle1);

    // Add a blue rectangle
    Rectangle *rect1 = new Rectangle(
        RectangleParams{20, 20, Color(0, 0, 255, 1.0f), 8, 6, true});
    testCollection->addShape(rect1);

    // Add a green circle
    Circle *circle2 =
        new Circle(CircleParams{35, 15, Color(0, 255, 0, 1.0f), 5, false});
    testCollection->addShape(circle2);

    collections.push_back(testCollection);
}

// Function to output pixels in a consistent format
void outputPixelsForComparison(const Pixels &pixels, const char *testName) {
    printf("=== %s ===\n", testName);
    printf("Total pixels: %zu\n", pixels.size());

    // Sort pixels for consistent output (x, then y)
    Pixels sorted = pixels;
    // Simple bubble sort (good enough for testing)
    for (size_t i = 0; i < sorted.size(); ++i) {
        for (size_t j = i + 1; j < sorted.size(); ++j) {
            if (sorted[j].x < sorted[i].x ||
                (sorted[j].x == sorted[i].x && sorted[j].y < sorted[i].y)) {
                std::swap(sorted[i], sorted[j]);
            }
        }
    }

    // Output first 50 pixels (or all if less)
    size_t maxOutput = (sorted.size() > 50) ? 50 : sorted.size();
    printf("First %zu pixels:\n", maxOutput);

    for (size_t i = 0; i < maxOutput; i++) {
        const Pixel &p = sorted[i];
        printf("Pixel[%zu]: (%d,%d) RGBA(%d,%d,%d,%.3f)\n", i, p.x, p.y,
               p.color.r, p.color.g, p.color.b, p.color.a);
    }

    if (sorted.size() > maxOutput) {
        printf("... and %zu more pixels\n", sorted.size() - maxOutput);
    }
    printf("Checksum: %zu pixels\n\n", sorted.size());
}

uint32_t calculatePixelChecksum(const Pixels &pixels) {
    uint32_t sum = 0;
    for (const auto &pixel : pixels) {
        sum += pixel.x;
        sum += pixel.y;
        sum += pixel.color.r;
        sum += pixel.color.g;
        sum += pixel.color.b;
        sum += static_cast<uint32_t>(pixel.color.a * 1000.0f);
    }
    return sum;
}
void runComparisonTest() {
    printf("Starting C++/TypeScript Comparison Test\n");
    printf("=======================================\n");

    const int width = 64;
    const int height = 64;
    Renderer renderer(width, height, Color(255, 255, 255, 1.0f));

    // Test 1: Simple static scene
    printf("TEST 1: Simple Static Scene\n");
    std::vector<Collection *> scene1;
    generateTestScene(scene1);

    DrawOptions options = {width, height, false}; // Aliased for consistency
    Pixels pixels1 = renderer.render(scene1, options);
    outputPixelsForComparison(pixels1, "Static Scene Aliased");
    printf("Calculated checksum: 0x%08lX\n", calculatePixelChecksum(pixels1));

    // Test 2: Same scene with anti-aliasing
    printf("TEST 2: Anti-Aliased Version\n");
    options.antialias = true;
    Pixels pixels2 = renderer.render(scene1, options);
    outputPixelsForComparison(pixels2, "Static Scene Anti-Aliased");
    printf("Calculated checksum: 0x%08lX\n", calculatePixelChecksum(pixels2));

    // Cleanup
    for (auto collection : scene1) {
        delete collection;
    }

    printf("Comparison test completed.\n");
    printf("Copy this output and compare with TypeScript version.\n");
}

extern "C" void app_main(void) {
    printf("C++/TypeScript Output Comparison\n");
    printf("================================\n");

    runComparisonTest();
}
