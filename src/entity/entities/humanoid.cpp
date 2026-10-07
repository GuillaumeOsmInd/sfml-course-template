// ****************************************************
// Created by Guillaume Boutigny on 10/6/2026.
// ****************************************************

#ifndef SFMLPROJECT_HUMANOID_H
#define SFMLPROJECT_HUMANOID_H

#include "../entity.h"

class Humanoid : public Entity {
public:
    Humanoid(int id, const sf::Texture &texture, sf::Vector2f vector2) : Entity(id, texture, vector2) {}

    // Mise a mort commune a tous les humanoides : plus de deplacement controle,
    // plus de collisions (les autres entites le traversent), et chute jusqu'a
    // sortir de sa zone. L'animation est propre a chacun : voir onDeath().
    // Sans effet si l'humanoide est deja mort.
    void die() {
        if (dead) return;
        dead = true;
        velocity = {0.f, 0.f};
        onDeath();
    }

    bool isDead() const { return dead; }

    // Mort, un humanoide ne bloque plus personne.
    bool isSolid() const override { return !dead; }

    // Saute si l'humanoide est au sol. Appelee par Player (touche Espace),
    // ou par une IA : la physique ne lit jamais le clavier elle-meme.
    void jump() {
        if (on_ground && !dead) {
            velocity.y = -JUMP_SPEED;
        }
    }

    void update(float dt, sf::View &view, const Game &game) override {
        Entity::update(dt, view, game);

        if (dead) {
            velocity.y = std::min(velocity.y + GRAVITY * dt, MAX_FALL_SPEED);
            // sprite.move et non move() : aucune collision, ni avec le niveau
            // ni avec les entites.
            sprite.move(velocity * dt);
            // Entierement sorti de sa zone par le bas : on le signale pour qu'il
            // soit retire du niveau. Mario, lui, fait redemarrer le niveau
            // (voir Game::updateThings).
            if (getHitbox().position.y >= (getZone() + 1) * ZONE_HEIGHT) {
                markFallenOutOfLevel();
            }
            return;
        }

        constexpr float SPEED = 100.f;          // pixels/s

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

        // Chute dans un trou : des que ses pieds FRANCHISSENT le bas de sa zone,
        // l'humanoide meurt. Il est encore visible, on voit donc son animation.
        // Le franchissement (avant au-dessus, apres en dessous) exclut le cas
        // d'une teleportation, qui ne passe pas par ici.
        const float zoneBottom = (getZone() + 1) * ZONE_HEIGHT;
        const float feetAfter = getHitbox().position.y + getHitbox().size.y;
        if (feetBefore < zoneBottom && feetAfter >= zoneBottom) {
            die();
        }
    }

protected:
    // Animation de mort, propre a chaque humanoide. Appelee une seule fois par
    // die(), apres que la vitesse a ete remise a zero : on peut y donner une
    // impulsion (petit saut de Mario) et changer le sprite.
    virtual void onDeath() {}

    // Au niveau de la classe : les classes derivees en ont aussi besoin.
    static constexpr float GRAVITY = 1000.f;       // pixels/s^2
    static constexpr float MAX_FALL_SPEED = 800.f; // evite de traverser le sol
    static constexpr float JUMP_SPEED = 375.f;     // impulsion initiale du saut, pixels/s

    static float approach(const float current, const float target, const float maxDelta) {
        if (current < target) return std::min(current + maxDelta, target);
        if (current > target) return std::max(current - maxDelta, target);
        return target;
    }

    sf::Vector2f dir;
    bool on_ground = false;
    bool dead = false;
};

// Humanoide qui marche tout seul et fait demi-tour contre les murs, comme les
// Goombas. Il part vers la gauche, c'est-a-dire vers le joueur qui arrive.
// Pour que le demi-tour retourne le sprite sur place, la classe derivee doit
// centrer l'origine du sprite (comme Player) : sinon setScale(-1) le decale
// de toute sa largeur.
class SmartHumanoid : public Humanoid {
public:
    SmartHumanoid(int id, const sf::Texture &texture, sf::Vector2f vector2) : Humanoid(id, texture, vector2) {
        dir.x = -1;
        facing_direction_ = LEFT;
    }

    // Pas besoin de redefinir update() : Humanoid avance deja selon dir.x.

    // Un mur a gauche ou a droite : demi-tour. Le sol et le plafond (DOWN, UP)
    // ne changent rien.
    void interactWithLevel(FacingDirection side) override {
        if (side == LEFT || side == RIGHT) {
            dir.x = -dir.x;
            facing_direction_ = dir.x < 0 ? LEFT : RIGHT;
        }
    }
};

#endif
