// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#include <algorithm>
#include <iostream>

#include "../entity.h"
#include "npc.cpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

class Player : public Entity {
public:
    explicit Player(const sf::Texture& texture) : Entity(0, texture, sf::Vector2f(0, 0)) {
        sprite.setTextureRect({{0, 8}, {16, 16}});
        sprite.setOrigin({8, 8});
    }

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

        float dirX = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
            dirX -= 1;
            facing_direction_ = LEFT;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            dirX += 1;
            facing_direction_ = RIGHT;
        }

        // On ne fixe plus la vitesse : on la rapproche de la vitesse voulue.
        // Touche relachee, la vitesse voulue est 0 et c'est la friction qui
        // ralentit le perso : forte au sol, tres faible en l'air.
        const float targetX = dirX * SPEED;
        float rate;
        if (dirX != 0) {
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

        if (dirX != 0) {
            sprite.setScale({ dirX, sprite.getScale().y });
        }

        if (on_ground) {
            if (dirX != 0) {
                sprite.setTextureRect({ {20 + (static_cast<int>(animTime / 0.1) % 3) * 18, 8}, {16, 16}});
            } else {
                sprite.setTextureRect({{0, 8}, {16, 16}});
            }
        } else {
            sprite.setTextureRect({ {96, 8}, {16, 16}});
        }

        view.setCenter(this->getPosition() + this->getSprite().getGlobalBounds().size / 2.f);
    }

    void interactWith(Entity &entity) const override {
        if (dynamic_cast<NPC*>(&entity) != nullptr) {
        }
    }

    template<typename Base, typename T>
    inline bool instanceof(const T *ptr) {
        return dynamic_cast<const Base*>(ptr) != nullptr;
    }

private:
    // Rapproche `current` de `target` d'au plus `maxDelta`, sans le depasser.
    static float approach(const float current, const float target, const float maxDelta) {
        if (current < target) return std::min(current + maxDelta, target);
        if (current > target) return std::max(current - maxDelta, target);
        return target;
    }

    bool on_ground = false;
};

