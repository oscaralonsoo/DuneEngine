cmake_minimum_required(VERSION 3.10)

project(DuneEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(DevIL REQUIRED)
find_package(assimp REQUIRED)

add_executable(DuneEngine 
    main.cpp
)

target_link_libraries(DuneEngine PRIVATE fmt::fmt SDL3::SDL3 glad::glad glm::glm imgui::imgui DevIL::IL assimp::assimp)

add_custom_command(TARGET DuneEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:DuneEngine>/resources)
