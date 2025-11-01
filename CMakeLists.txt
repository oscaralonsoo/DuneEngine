cmake_minimum_required(VERSION 3.10)

project(DuneEngine)

find_package(fmt CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(DuneEngine 
    main.cpp
    stb_image.cpp
)

target_link_libraries(DuneEngine PRIVATE fmt::fmt SDL3::SDL3 glad::glad glm::glm imgui::imgui)

add_custom_command(TARGET DuneEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:DuneEngine>/resources)
