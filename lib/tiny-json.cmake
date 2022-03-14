# Add library cpp files
add_library(tiny_json INTERFACE)
target_sources(tiny_json INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/tiny-json/tiny-json.c
)

# Add include directory
target_include_directories(tiny_json INTERFACE ${CMAKE_CURRENT_LIST_DIR}/tiny-json/)

# Add the standard library to the build
target_link_libraries(json_maker INTERFACE pico_stdlib)