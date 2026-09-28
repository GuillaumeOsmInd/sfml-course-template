// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_GAME_H
#define SFMLPROJECT_GAME_H
#include <memory>
#include <vector>

#include "entity/entity.h"
#include "level/level.h"

class Game {
public:
    Game();

    void initialize();

    void updateThings();
    void drawThings();

    void loop();

    void onResize(sf::Vector2u size);

    sf::View& getView();
    void setView(const sf::View &view);

    void addEntity(std::shared_ptr<Entity> entity);

    bool hasEntity(std::shared_ptr<Entity> entity);

    Entity* getEntity(long int id);
    bool removeEntity(const Entity *entity);
    bool removeEntity(long int id);

    bool isFree(const sf::FloatRect &rect, Entity &ignore) const;

private:
    sf::RenderWindow window;
    // Camera : modifiable par les entites pendant update(), appliquee a la
    // fenetre juste avant le rendu.
    sf::View view;
    sf::Clock clock;
    std::vector<std::shared_ptr<Entity>> entities{};
    Level* currentLevel;
    std::vector<std::unique_ptr<Level>> levels{};
};

#endif //SFMLPROJECT_GAME_H
