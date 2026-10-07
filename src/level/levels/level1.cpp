// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#include "../level.h"
#include "../../game.h"

class Level1 : public Level {
public:
    Level1() : Level(RESOURCES_DIR "/textures/levels/-1/-1.png") {
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
