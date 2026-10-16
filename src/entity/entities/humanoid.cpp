// ****************************************************
// Created by Guillaume Boutigny on 10/6/2026.
// ****************************************************

#ifndef SFMLPROJECT_HUMANOID_H
#define SFMLPROJECT_HUMANOID_H

#include "../entity.h"
#include "../../level/zone.h"

class Humanoid : public Entity {
public:
    Humanoid(const sf::Texture &texture, sf::Vector2f vector2) : Entity(texture, vector2) {}

    // Hitbox un peu plus petite que le sprite, pour passer dans les ouvertures
    // qui ont exactement sa taille :
    //  - HITBOX_INSET_X de chaque cote : un trou d'un bloc de large (16 px) ;
    //  - HITBOX_INSET_TOP en haut : une porte de 32 px de haut pour le grand
    //    Mario (sa hitbox de 32 px toucherait le mur au-dessus).
    // Le bas ne change pas : les pieds restent exactement au bord du sprite.
    sf::FloatRect getHitbox() const override {
        sf::FloatRect box = Entity::getHitbox();
        box.position.x += HITBOX_INSET_X;
        box.size.x -= 2.f * HITBOX_INSET_X;
        box.position.y += HITBOX_INSET_TOP;
        box.size.y -= HITBOX_INSET_TOP;
        return box;
    }

    void die() {
        if (dead) return;
        dead = true;
        velocity = {0.f, 0.f};
        onDeath();
    }

    bool isDead() const { return dead; }

    bool isSolid() const override { return !dead; }

    void jump() {
        if (on_ground && !dead) {
            velocity.y = -JUMP_SPEED;
        }
    }

    void update(float dt, sf::View &view, const Game &game) override {
        Entity::update(dt, view, game);

        if (dead) {
            velocity.y = std::min(velocity.y + GRAVITY * dt, MAX_FALL_SPEED);

            sprite.move(velocity * dt);

            if (getHitbox().position.y >= (getZone() + 1) * ZONE_HEIGHT) {
                markFallenOutOfLevel();
            }
            return;
        }


        constexpr float GROUND_ACCEL = 700.f;   // pleine vitesse en ~0.08 s
        constexpr float GROUND_FRICTION = 400.f; // arret en ~0.1 s quand on lache
        constexpr float AIR_ACCEL = 400.f;       // controle reduit en l'air
        constexpr float AIR_FRICTION = 50.f;     // l'elan se conserve en l'air

        const float targetX = dir.x * walk_speed;
        float rate;
        if (dir.x != 0) {
            rate = on_ground ? GROUND_ACCEL : AIR_ACCEL;
        } else {
            rate = on_ground ? GROUND_FRICTION : AIR_FRICTION;
        }
        velocity.x = approach(velocity.x, targetX, rate * dt);

        velocity.y = std::min(velocity.y + GRAVITY * dt, MAX_FALL_SPEED);

        const float feetBefore = getHitbox().position.y + getHitbox().size.y;

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

        const float zoneBottom = (getZone() + 1) * ZONE_HEIGHT;
        const float feetAfter = getHitbox().position.y + getHitbox().size.y;
        if (feetBefore < zoneBottom && feetAfter >= zoneBottom) {
            die();
        }
    }

protected:
    virtual void onDeath() {}

    static constexpr float HITBOX_INSET_X = 2.f;   // pixels retires a gauche et a droite de la hitbox
    static constexpr float HITBOX_INSET_TOP = 2.f; // pixels retires en haut de la hitbox
    static constexpr float GRAVITY = 1000.f;       // pixels/s^2
    static constexpr float MAX_FALL_SPEED = 800.f; // evite de traverser le sol
    static constexpr float JUMP_SPEED = 375.f;     // impulsion initiale du saut, pixels/s

    static float approach(const float current, const float target, const float maxDelta) {
        if (current < target) return std::min(current + maxDelta, target);
        if (current > target) return std::max(current - maxDelta, target);
        return target;
    }

    sf::Vector2f dir;
    float walk_speed = 100.f;   // pixels/s (Goomba : plus lent)
    bool on_ground = false;
    bool dead = false;
};

class SmartHumanoid : public Humanoid {
public:
    SmartHumanoid(const sf::Texture &texture, sf::Vector2f vector2) : Humanoid(texture, vector2) {
        dir.x = -1;
        facing_direction_ = LEFT;
    }

    void interactWithLevel(FacingDirection side) override {
        if (side == LEFT || side == RIGHT) {
            dir.x = -dir.x;
            facing_direction_ = dir.x < 0 ? LEFT : RIGHT;
        }
    }
};

#endif
