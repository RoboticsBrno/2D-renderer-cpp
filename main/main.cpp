#include "TextureTest.hpp"
#include "examples/CollisionTest.hpp"
#include "examples/Fire.hpp"
#include "examples/LittlefsTest.hpp"
#include "examples/Platformer.hpp"
#include "examples/SolarSystem.hpp"
#include "examples/SpaceShooter.hpp"
#include "examples/TestShapes.hpp"
#include "examples/TextExample.hpp"

extern "C" int app_main(void) {
    // To run an example, uncomment it and comment out the others
    // runSolarSystem();
    // runTestShapes();
    // runCollisionTest();
    // runFire();
    // runPlatformer();
    runSpaceShooter();
    // runLittlefsTest();
    // runTextureTest();
    return 0;
}
