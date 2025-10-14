cmake_minimum_required(VERSION 3.10)

project(GameEngine)

find_package(fmt CONFIG REQUIRED)

find_package(sdl3 CONFIG REQUIRED)

add_executable(GameEngine gameEngine.cpp)

target_link_libraries(GameEngine PRIVATE fmt::fmt
GameEngine PRIVATE sdl3::sdl3)