// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************


#ifndef SFMLPROJECT_ENEMY_H
#define SFMLPROJECT_ENEMY_H

#include "../humanoid.cpp"
#include "../blocks/block.cpp"

// Ennemi : marche tout seul et fait demi-tour contre les murs (SmartHumanoid).
// Toucher Mario de cote le tue, Mario qui lui tombe dessus l'ecrase : c'est
// gere dans Player::interactWith.
class Enemy : public SmartHumanoid {
public:
    explicit Enemy(const sf::Texture &texture) : SmartHumanoid(texture, sf::Vector2f(0, 0)) {
    }

    // Mario lui saute dessus. Par defaut : mort commune des humanoides (il
    // tombe a travers le sol). Les ennemis la redefinissent (Goomba : ecrase).
    virtual void stomp() {
        die();
    }

    // Demi-tour contre un bloc ou un autre ennemi, comme contre un mur. Les
    // entites non solides (piece, ennemi ecrase...) ne comptent pas.
    void interactWith(Entity &entity, const FacingDirection side) override {
        if ((side == LEFT || side == RIGHT) && entity.isSolid()
            && (dynamic_cast<Block *>(&entity) != nullptr || dynamic_cast<Enemy *>(&entity) != nullptr)) {
            dir.x = -dir.x;
            facing_direction_ = dir.x < 0 ? LEFT : RIGHT;
        }
    }
};

#endif
