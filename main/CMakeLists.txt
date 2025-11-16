idf_component_register(SRCS 
    "main.cpp"
    "src/Renderer.cpp"
    "src/Utils.cpp"
    "src/Texture.cpp"
    "src/Collider.cpp"
    "src/Shapes/Shape.cpp"
    "src/Shapes/Circle.cpp"
    "src/Shapes/Rectangle.cpp"
    "src/Shapes/Polygon.cpp"
    "src/Shapes/LineSegment.cpp"
    "src/Shapes/Point.cpp"
    "src/Shapes/RegularPolygon.cpp"
    "src/Shapes/Collection.cpp"
    INCLUDE_DIRS 
    "include"
    "include/Shapes"
    REQUIRES ESP32-HUB75-MatrixPanel-I2S-DMA
    PRIV_REQUIRES esp_timer)

target_compile_options(${COMPONENT_LIB} PRIVATE 
    -Wno-narrowing
    -Wno-error=narrowing)
