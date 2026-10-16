// ****************************************************
// Created by Guillaume Boutigny on 10/16/2026.
// ****************************************************

#ifndef SFMLPROJECT_MUSHROOM_H
#define SFMLPROJECT_MUSHROOM_H

#include "../humanoid.cpp"
#include "../blocks/block.cpp"
#include "items_texture.h"

// Champignon : sort d'un bloc mystere, avance tout seul vers la droite et fait
// demi-tour contre les murs et les blocs. Mario le ramasse au contact et
// devient grand (voir Player::grow). Non solide : les ennemis le traversent.
class Mushroom : public SmartHumanoid {
public:
    // `cell` : coin haut-gauche de la case ou il apparait.
    explicit Mushroom(const sf::Vector2f cell) : SmartHumanoid(itemsTexture(), {0, 0}) {
        sprite.setTextureRect({{0, 8}, {16, 16}});
        sprite.setOrigin({8, 8});                      // demi-tour sur place
        setPosition(cell + sf::Vector2f(8.f, 8.f));
        setVisible(true);
        dir.x = 1;                                     // part vers la droite
        facing_direction_ = RIGHT;
        walk_speed = 60.f;
    }

    bool isSolid() const override { return false; }

    // Demi-tour contre un bloc, comme contre un mur.
    void interactWith(Entity &entity, const FacingDirection side) override {
        if ((side == LEFT || side == RIGHT) && entity.isSolid() && dynamic_cast<Block *>(&entity) != nullptr) {
            dir.x = -dir.x;
            facing_direction_ = dir.x < 0 ? LEFT : RIGHT;
        }
    }
};

#endif
