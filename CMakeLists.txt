cmake_minimum_required(VERSION 3.10)

project(2D-renderer-cpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")

add_executable(2D-renderer-cpp ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(2D-renderer-cpp PRIVATE src)

# Compiler options (optional)
target_compile_options(2D-renderer-cpp PRIVATE -Wall -Wextra)

# For better build organization in IDEs
source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
