cmake_minimum_required(VERSION 4.1)
project(OSFinalProject LANGUAGES C)

set(RAYLIB_VERSION 5.5)
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)

    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
    endif ()
endif ()

set(CMAKE_C_STANDARD 11)

add_executable(OSFinalProject src/app/main.c
        src/core/common.h
        src/core/graph.c
        src/core/graph.h
        src/render/draw.h
        src/render/draw.c)
target_include_directories(OSFinalProject PUBLIC src)
target_link_libraries(OSFinalProject raylib)
configure_file(data/sample_graph.dat data/sample_graph.dat COPYONLY)
