cmake_minimum_required(VERSION 3.28)
project(SFMLProject LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(main
        src/game.cpp
        src/game.h
        src/entity/entity.cpp
        src/entity/entity.h
        src/level/level.cpp
        src/level/level.h
        src/entity/entities/player.cpp
        src/level/levels/level1.cpp
)
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics SFML::Audio)

target_compile_definitions(main PRIVATE RESOURCES_DIR="${CMAKE_SOURCE_DIR}/ressources")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)

add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/ressources"
        "$<TARGET_FILE_DIR:main>/ressources"
        COMMENT "Copie des ressources")