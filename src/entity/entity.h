// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_ENTITY_H
#define SFMLPROJECT_ENTITY_H

#include <utility>

#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/View.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Window.hpp"

#include "facing_direction.h"
#include "../level/zone.h"

class Game;

class Entity {
public:

    explicit Entity(int id, const sf::Texture &texture, sf::Vector2f position);
    virtual ~Entity() = default;

    virtual void update(float dt, sf::View &view, const Game &game) {
        this->animTime += dt;
    }

    virtual void draw(sf::RenderTarget &target, const sf::RenderStates &states) const;

    long int getId() const;

    sf::FloatRect getHitbox() const;

    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f position);

    int getZone() const;
    std::pair<bool, bool> move(sf::Vector2f offset, const Game& game);

    sf::Angle getRotation() const;
    void setRotation(sf::Angle rotation);

    sf::Vector2f getScale() const;
    void setScale(sf::Vector2f scale);

    const sf::Sprite& getSprite() const;
    void setTexture(const sf::Texture &texture);

    bool isVisible() const;
    void setVisible(bool visible);

    virtual void interactWith(Entity &entity, FacingDirection side) const;

    virtual void interactWithLevel(FacingDirection side);

    // Faux = les autres entites la traversent (ex. Mario mort).
    virtual bool isSolid() const { return true; }

    bool hasFallenOutOfLevel() const;
    void markFallenOutOfLevel();

protected:
    long int id;
    sf::Sprite sprite;
    bool is_visible = false;
    float animTime = 0.;
    FacingDirection facing_direction_ = DOWN;
    sf::Vector2f velocity, last_velocity;
    bool fallen_out_of_level = false;
    int zone = 0;
};
#endif