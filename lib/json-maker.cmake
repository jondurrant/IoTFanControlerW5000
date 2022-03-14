# Add library cpp files
add_library(json_maker STATIC)
target_sources(json_maker PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/json-maker/src/json-maker.c
)

# Add include directory
target_include_directories(json_maker PUBLIC ${CMAKE_CURRENT_LIST_DIR}/json-maker/src/include)

# Add the standard library to the build
target_link_libraries(json_maker PUBLIC pico_stdlib)