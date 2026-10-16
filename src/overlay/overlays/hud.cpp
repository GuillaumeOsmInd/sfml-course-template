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

        // Pieces, en haut a droite, sur deux chiffres comme dans le jeu original.
        add<OverlayText>(game.getFont(), sf::Vector2f(ZONE_HEIGHT - 16.f, 12.f), "", 8, Align::RIGHT)
            .bindTo([](const Game &g) {
                const int coins = g.getCoins();
                return sf::String(U"PIÈCES × ") + sf::String((coins < 10 ? "0" : "") + std::to_string(coins));
            });
    }

    bool isVisible(const Game &game) const override {
        return game.getCurrentLevel() != nullptr && game.getCurrentLevel()->needHud();
    }
};

#endif
