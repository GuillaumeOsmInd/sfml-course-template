// ****************************************************
// Created by Guillaume Boutigny on 10/6/2026.
// ****************************************************

#ifndef SFMLPROJECT_HUMANOID_H
#define SFMLPROJECT_HUMANOID_H

#include "../entity.h"

class Humanoid : public Entity {
public:
    Humanoid(int id, const sf::Texture &texture, sf::Vector2f vector2) : Entity(id, texture, vector2) {}

    void update(float dt, sf::View &view, const Game &game) override {
        Entity::update(dt, view, game);

        constexpr float SPEED = 100.f;          // pixels/s
        constexpr float GRAVITY = 1000.f;       // pixels/s^2
        constexpr float JUMP_SPEED = 375.f;     // impulsion initiale, pixels/s
        constexpr float MAX_FALL_SPEED = 800.f; // evite de traverser le sol

        constexpr float GROUND_ACCEL = 700.f;   // pleine vitesse en ~0.08 s
        constexpr float GROUND_FRICTION = 400.f; // arret en ~0.1 s quand on lache
        constexpr float AIR_ACCEL = 400.f;       // controle reduit en l'air
        constexpr float AIR_FRICTION = 50.f;     // l'elan se conserve en l'air

        const float targetX = dir.x * SPEED;
        float rate;
        if (dir.x != 0) {
            rate = on_ground ? GROUND_ACCEL : AIR_ACCEL;
        } else {
            rate = on_ground ? GROUND_FRICTION : AIR_FRICTION;
        }
        velocity.x = approach(velocity.x, targetX, rate * dt);

        if (on_ground && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            velocity.y = -JUMP_SPEED;
        }

        velocity.y = std::min(velocity.y + GRAVITY * dt, MAX_FALL_SPEED);

        const auto [movedX, movedY] = this->move(velocity * dt, game);

        on_ground = !movedY && velocity.y > 0;
        if (!movedY) {
            velocity.y = 0;
        }
        if (!movedX) {
            velocity.x = 0;
        }

        if (dir.x != 0) {
            sprite.setScale({ dir.x, sprite.getScale().y });
        }
    }

protected:
    static float approach(const float current, const float target, const float maxDelta) {
        if (current < target) return std::min(current + maxDelta, target);
        if (current > target) return std::max(current - maxDelta, target);
        return target;
    }

    sf::Vector2f dir;
    bool on_ground = false;
};

#endif
