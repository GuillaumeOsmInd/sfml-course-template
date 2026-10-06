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

    void load(Game &game) {}
    void unload() {}

    void update(float dt);
    void draw(sf::RenderWindow &window, RenderPass renderPass) const;

    const sf::Sprite& getBackground() const;
    const sf::Sprite& getForeground() const;

    sf::Vector2f getSize() const;

    virtual sf::Vector2f getSpawnPoint() const;

    // Vrai si l'entite qui passe de `from` a `to` en avancant vers `side`
    // est bloquee par le niveau : mur (rouge impair) dans toutes les
    // directions, plateforme traversable (bleu impair) seulement par le dessus.
    bool isCollinding(const sf::FloatRect &from, const sf::FloatRect &to, FacingDirection side) const;

    // Vrai si ce deplacement fait FRANCHIR le bord du bas du niveau, de
    // l'interieur vers l'exterieur. Une entite deja en dessous (sous-terrain
    // atteint par teleportation) ne le franchit pas.
    bool fallsOutOfBottom(const sf::FloatRect &from, const sf::FloatRect &to) const;


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
