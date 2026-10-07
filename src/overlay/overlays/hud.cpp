// ****************************************************
// Created by Guillaume Boutigny on 10/7/2026.
// ****************************************************

#ifndef SFMLPROJECT_HUD_H
#define SFMLPROJECT_HUD_H

#include <string>

#include "../overlay.h"
#include "../../game.h"

class HUD : public Overlay {
public:
    explicit HUD(const Game &game) {
        add<OverlayText>(game.getFont(), sf::Vector2f(16.f, 12.f))
            .bindTo([](const Game &g) {
                return sf::String(U"MARIO × ") + sf::String(std::to_string(g.getLife()));
            });
    }

    bool isVisible(const Game &game) const override {
        return game.getCurrentLevel() != nullptr && game.getCurrentLevel()->needHud();
    }
};

#endif
