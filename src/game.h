// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_GAME_H
#define SFMLPROJECT_GAME_H
#include <memory>
#include <vector>

#include "entity/entity.h"
#include "level/level.h"
#include "overlay/overlay.h"
#include "SFML/Graphics/Font.hpp"

class Game {
public:
    Game();

    void initialize();

    void updateThings();
    void drawThings();

    void loop();

    void onResize(sf::Vector2u size);

    void changeLevel(std::size_t index);

    sf::View& getView();
    void setView(const sf::View &view);

    bool isFree(const sf::FloatRect &from, const sf::FloatRect &to, Entity &ent, FacingDirection side) const;

private:
    void updateViewport();
    void clampView();

    sf::RenderWindow window;
    sf::View view;

    sf::Clock clock;

    Level* currentLevel = nullptr;

    std::weak_ptr<Entity> player;
    std::size_t currentLevelIndex = 0;
    sf::Texture playerTexture;
    int life = 3;

    std::vector<std::unique_ptr<Level>> levels{};
    std::vector<std::shared_ptr<Entity>> entities{};
    std::vector<std::shared_ptr<Overlay>> overlays{};

    sf::Font font = sf::Font(RESOURCES_DIR "/fonts/smb.ttf");
};

#endif
