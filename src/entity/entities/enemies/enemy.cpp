// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************


#ifndef SFMLPROJECT_ENEMY_H
#define SFMLPROJECT_ENEMY_H

#include "../humanoid.cpp"

class Enemy : public Humanoid {
public:
    explicit Enemy(const int id, const sf::Texture &texture) : Humanoid(id, texture, sf::Vector2f(0, 0)) {
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

    }

    void interactWith(Entity &entity, FacingDirection side) const override {}
};

#endif
