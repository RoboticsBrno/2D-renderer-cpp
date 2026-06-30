cmake_minimum_required(VERSION 3.16)
project(renderer-desktop CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RENDERER_SOURCES
    ../src/Profiler.cpp
    ../src/Renderer.cpp
    ../src/Utils.cpp
    ../src/Texture.cpp
    ../src/Collider.cpp
    ../src/Font.cpp
    ../src/DrawUtils.cpp
    ../src/Shapes/Shape.cpp
    ../src/Shapes/Circle.cpp
    ../src/Shapes/Rectangle.cpp
    ../src/Shapes/Polygon.cpp
    ../src/Shapes/LineSegment.cpp
    ../src/Shapes/Point.cpp
    ../src/Shapes/RegularPolygon.cpp
    ../src/Shapes/Collection.cpp
)

add_library(renderer STATIC ${RENDERER_SOURCES})
target_include_directories(renderer PUBLIC
    ../include
    ../include/Shapes
    ../include/Font
)
target_compile_options(renderer PRIVATE -Wall -Wno-narrowing)

add_executable(renderer-test main.cpp)
target_link_libraries(renderer-test renderer)
