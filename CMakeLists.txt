cmake_minimum_required(VERSION 3.22)
project(Planes CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adding Raylib
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/sources/*.cpp") # Define PROJECT_SOURCES as a list of all source files
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/") # Define PROJECT_INCLUDE to be the path to the include directory of the project

# Declaring our executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

message(STATUS "Compiler id = ${CMAKE_CXX_COMPILER_ID}")
if(EMSCRIPTEN)
    # also copy our index.html file for wasm builds
    add_custom_target(
        index_file
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/sources/index.html
            ${CMAKE_BINARY_DIR}/index.html)
    add_dependencies(${PROJECT_NAME} index_file)

    target_link_options(
        ${PROJECT_NAME} PRIVATE
        --preload-file ../assets
        -sEXPORTED_FUNCTIONS=_EnableSound,_main
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap)

    target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./assets/")
    target_compile_definitions(${PROJECT_NAME} PUBLIC EMSCRIPTEN=1) # Define EMCC macro for emscripten
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
endif()

