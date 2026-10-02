// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#include "../level.h"
#include "../../game.h"

class Level1 : public Level {
public:
    Level1() : Level(RESOURCES_DIR "/levels/-1/-1.png") {

        // backgroundTexture.setRepeated(true);
        // backgroundTexture.setSmooth(true);

        // background.setTextureRect(sf::IntRect({0, 0}, {500, 500}));
        // background.setScale(sf::Vector2f(2.f, 2.f));
        // background.setPosition(sf::Vector2f(0.f, 0.f));
    }

    void load(Game &game) {
        // npcTexture = sf::Texture(RESOURCES_DIR "/player/npc_sprite_sheet.png");
        // auto npc = std::make_shared<NPC>(1, npcTexture);
        // npc->setPosition({250.f, 250.f});
        // npc->setVisible(true);
        // game.addEntity(npc);
    }

    sf::Vector2f getSpawnPoint() const override {
        return sf::Vector2f(17 + 8, 191);
    }

private:
    sf::Texture npcTexture;
};
