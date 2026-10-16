// ****************************************************
// Created by Guillaume Boutigny on 10/8/2026.
// ****************************************************

#ifndef SFMLPROJECT_COIN_H
#define SFMLPROJECT_COIN_H

#include "../../entity.h"
#include "items_texture.h"

class CoinPop : public Entity {
public:
    explicit CoinPop(const sf::Vector2f cell) : Entity(itemsTexture(), {0, 0}) {
        sprite.setTextureRect({{180, 36}, {8, 16}});
        setPosition(cell + sf::Vector2f(4.f, 0.f));
        startY = getPosition().y;
        velocity = {0.f, -JUMP_SPEED};
        setVisible(true);
    }

    bool isSolid() const override { return false; }

    void update(const float dt, sf::View &view, const Game &game) override {
        Entity::update(dt, view, game);
        velocity.y += GRAVITY * dt;
        sprite.move(velocity * dt);   // sans collisions
        sprite.setTextureRect({{180 + (static_cast<int>(animTime / 0.05f) % 4) * 10, 36}, {8, 16}});

        if (velocity.y > 0 && getPosition().y >= startY) {
            markForRemoval();
        }
    }

private:
    static constexpr float JUMP_SPEED = 250.f;   // pixels/s : monte d'environ 30 px
    static constexpr float GRAVITY = 1000.f;     // pixels/s^2

    float startY = 0.f;
};

#endif
