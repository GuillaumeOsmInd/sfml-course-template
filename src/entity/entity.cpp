// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************
#include "entity.h"

#include "../game.h"

Entity::Entity(const int id, const sf::Texture &texture, const sf::Vector2f position)
    : id(id), sprite(texture) {
    this->setPosition(position);
}

void Entity::draw(sf::RenderTarget &target, const sf::RenderStates &states) const {
    target.draw(this->sprite, states);
}

long int Entity::getId() const {
    return this->id;
}

const sf::Sprite& Entity::getSprite() const {
    return this->sprite;
}

sf::Vector2f Entity::getPosition() const {
    return this->sprite.getPosition();
}

void Entity::setPosition(const sf::Vector2f position) {
    this->sprite.setPosition(position);
}

sf::FloatRect Entity::getHitbox() const {
    return this->sprite.getGlobalBounds();
}

void Entity::interactWith(Entity &entity) const {}

std::pair<bool, bool> Entity::move(const sf::Vector2f offset, const Game& game) {
    const sf::Vector2f start = getPosition();
    bool movedX = false;
    bool movedY = false;

    if (offset.x != 0) {
        setPosition({start.x + offset.x, start.y});
        movedX = game.isFree(getHitbox(), *this);
        if (!movedX) {
            setPosition(start);
        }
    }

    if (offset.y != 0) {
        const float x = getPosition().x;
        // Hitbox avant le deplacement vertical : les plateformes traversables
        // en ont besoin pour savoir si on arrive par le dessus.
        const sf::FloatRect before = getHitbox();
        setPosition({x, start.y + offset.y});
        movedY = game.isFree(getHitbox(), *this, offset.y > 0 ? &before : nullptr);
        if (!movedY) {
            setPosition({x, start.y});
        }
    }

    return {movedX, movedY};
}

sf::Angle Entity::getRotation() const {
    return this->sprite.getRotation();
}

void Entity::setRotation(const sf::Angle angle) {
    this->sprite.setRotation(angle);
}

sf::Vector2f Entity::getScale() const {
    return this->sprite.getScale();
}

void Entity::setScale(const sf::Vector2f scale) {
    this->sprite.setScale(scale);
}

void Entity::setTexture(const sf::Texture &texture) {
    this->sprite.setTexture(texture, true);
}

bool Entity::isVisible() const {
    return this->is_visible;
}

void Entity::setVisible(const bool visible) {
    this->is_visible = visible;
}
