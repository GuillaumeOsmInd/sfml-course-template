// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#include "../level.h"
#include "../../game.h"
#include "../../entity/entities/enemies/goomba.cpp"

class Level1_1 : public Level {
public:
    Level1_1() : Level(RESOURCES_DIR "/textures/levels/-1/-1.png") {}

    void load(Game &game) override {
        addEntity(std::make_shared<Goomba>(sf::Vector2f(496, 192 - 16)));
    }
};

class Level1_2 : public Level {
public:
    Level1_2() : Level(RESOURCES_DIR "/textures/levels/1-1/1-1.png") {

    }
};