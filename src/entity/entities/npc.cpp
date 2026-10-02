// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************


#ifndef SFMLPROJECT_NPC_H
#define SFMLPROJECT_NPC_H

#include "../entity.h"

class NPC : public Entity {
public:
    explicit NPC(const int id, const sf::Texture &texture) : Entity(id, texture, sf::Vector2f(0, 0)) {
    }

    void update(float dt, sf::View &view, const Game &game) override {
        Entity::update(dt, view, game);
        const float step = 100.f * dt;

        facing_direction_ = static_cast<FacingDirection>((static_cast<int>(animTime) / 1) % 4);
        sf::Vector2f dir;

        if (facing_direction_ == LEFT) {
            dir.x = -1;
        } else if (facing_direction_ == RIGHT) {
            dir.x = 1;
        } else if (facing_direction_ == UP) {
            dir.y = -1;
        } else if (facing_direction_ == DOWN) {
            dir.y = 1;
        }

        int y = static_cast<int>(facing_direction_) * 27;
        const auto [movedX, movedY] = this->move(dir * step, game);
        if (movedX || movedY) {
            sprite.setTextureRect(sf::IntRect({(static_cast<int>(animTime / 0.15) % 4) * 19, y}, {19, 27}));
        } else {
            sprite.setTextureRect(sf::IntRect({0, y}, {19, 27}));
        }
    }

    void interactWith(Entity &entity) const override {}
};

#endif
