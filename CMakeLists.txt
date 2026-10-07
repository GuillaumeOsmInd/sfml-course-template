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

# SFML 3.1 compile FreeType et HarfBuzz lui-meme, mais ne declare que le lien
# HarfBuzz -> FreeType. FreeType appelle pourtant HarfBuzz (auto-hinting) : en
# lien statique avec MinGW, libharfbuzz passe avant libfreetype et les hb_*
# restent non resolus des qu'on utilise sf::Font. On declare le lien inverse :
# CMake voit le cycle et repete les bibliotheques sur la ligne de lien.
if(TARGET freetype AND TARGET harfbuzz)
    target_link_libraries(freetype INTERFACE harfbuzz)
endif()

add_executable(main
        src/game.cpp
        src/game.h
        src/entity/entity.cpp
        src/entity/entity.h
        src/level/level.cpp
        src/level/level.h
        src/entity/entities/player/player.cpp
        src/level/levels/level1.cpp
        src/entity/entities/humanoid.cpp
        src/entity/entities/blocks/block.cpp
        src/overlay/overlay.h
        src/overlay/overlay.cpp
        src/overlay/overlays/hud.cpp
        src/overlay/overlays/game_over.cpp
        src/level/levels/menu.cpp
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