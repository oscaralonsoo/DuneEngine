cmake_minimum_required(VERSION 3.10)

project(DuneEngine)

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(DuneEngine main.cpp)

target_link_libraries(DuneEngine PRIVATE fmt::fmt SDL3::SDL3 glad::glad glm::glm imgui::imgui)