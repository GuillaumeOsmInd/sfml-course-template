// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_LEVEL_H
#define SFMLPROJECT_LEVEL_H

#include <filesystem>

#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "../entity/facing_direction.h"
#include "zone.h"

class Entity;
class Game;

enum RenderPass {
    BEFORE_ENTITIES,
    AFTER_ENTITIES
};

class Level {
public:
    explicit Level(const std::filesystem::path &backgroundFile);
    virtual ~Level() = default;

    virtual void load(Game &game) {}
    virtual void unload() {}

    void update(float dt);
    void draw(sf::RenderWindow &window, RenderPass renderPass) const;

    const sf::Sprite& getBackground() const;
    const sf::Sprite& getForeground() const;
    virtual bool needHud() const { return true; }

    sf::Vector2f getSize() const;

    virtual sf::Vector2f getSpawnPoint() const;

    bool isCollinding(const sf::FloatRect &from, const sf::FloatRect &to, FacingDirection side, int zone) const;

    bool fallsOutOfBottom(const sf::FloatRect &from, const sf::FloatRect &to, int zone) const;

    void addEntity(std::shared_ptr<Entity> entity);
    bool hasEntity(std::shared_ptr<Entity> entity);

    Entity* getEntity(long int id);
    bool removeEntity(const Entity *entity);
    bool removeEntity(long int id);

    std::vector<std::shared_ptr<Entity>> &getEntities();
    const std::vector<std::shared_ptr<Entity>> &getEntities() const;
protected:
    sf::Texture backgroundTexture;
    sf::Sprite background;

    sf::Texture foregroundTexture;
    sf::Sprite foreground;

    sf::Image collisionMask;

    std::vector<std::shared_ptr<Entity>> entities{};
};


#endif
