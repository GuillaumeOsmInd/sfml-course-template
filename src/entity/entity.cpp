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
    this->zone = zoneAt(getHitbox().getCenter().y);
}

int Entity::getZone() const {
    return this->zone;
}

sf::FloatRect Entity::getHitbox() const {
    return this->sprite.getGlobalBounds();
}

void Entity::interactWith(Entity &entity, FacingDirection side) const {}

void Entity::interactWithLevel(FacingDirection side) {}

bool Entity::hasFallenOutOfLevel() const {
    return this->fallen_out_of_level;
}

void Entity::markFallenOutOfLevel() {
    this->fallen_out_of_level = true;
}

std::pair<bool, bool> Entity::move(const sf::Vector2f offset, const Game& game) {
    const sf::Vector2f start = getPosition();
    bool movedX = false;
    bool movedY = false;

    // Pour chaque axe : hitbox avant, deplacement, puis test entre les deux.
    // sprite.setPosition et non setPosition : se deplacer ne change pas de zone.
    if (offset.x != 0) {
        const sf::FloatRect before = getHitbox();
        sprite.setPosition({start.x + offset.x, start.y});
        const FacingDirection side = offset.x > 0 ? RIGHT : LEFT;
        movedX = game.isFree(before, getHitbox(), *this, side);
        if (!movedX) {
            sprite.setPosition(start);
        }
    }

    if (offset.y != 0) {
        const float x = getPosition().x;
        const sf::FloatRect before = getHitbox();
        sprite.setPosition({x, start.y + offset.y});
        const FacingDirection side = offset.y > 0 ? DOWN : UP;
        movedY = game.isFree(before, getHitbox(), *this, side);
        if (!movedY) {
            sprite.setPosition({x, start.y});
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
