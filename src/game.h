// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_GAME_H
#define SFMLPROJECT_GAME_H
#include <memory>
#include <vector>

#include "entity/entity.h"
#include "level/level.h"
#include "entity/entities/player.cpp"
#include "entity/entities/npc.cpp"

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

    bool isFree(const sf::FloatRect &rect, Entity &ent, const sf::FloatRect *from = nullptr) const;

private:
    void updateViewport();
    void clampView();

    sf::RenderWindow window;
    sf::View view;
    sf::Clock clock;
    Level* currentLevel = nullptr;
    sf::Texture playerTexture;
    std::vector<std::unique_ptr<Level>> levels{};
};

#endif
