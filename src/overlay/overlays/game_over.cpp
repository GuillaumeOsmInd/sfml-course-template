// ****************************************************
// Created by Guillaume Boutigny on 10/7/2026.
// ****************************************************

#ifndef SFMLPROJECT_GAME_OVER_H
#define SFMLPROJECT_GAME_OVER_H

#include "SFML/Graphics/RectangleShape.hpp"

#include "../overlay.h"
#include "../../game.h"

class GameOverOverlay : public Overlay {
public:
    explicit GameOverOverlay(const Game &game) {
        add<OverlayText>(game.getFont(), sf::Vector2f(ZONE_HEIGHT / 2.f, ZONE_HEIGHT / 2.f - 4.f),
                         "GAME OVER", 8, Align::CENTER);
    }

    bool isVisible(const Game &game) const override {
        return game.isGameOver();
    }

    void draw(const Game &game, sf::RenderTarget &target) const override {
        sf::RectangleShape background({ZONE_HEIGHT, ZONE_HEIGHT});
        background.setFillColor(sf::Color::Black);
        target.draw(background);

        Overlay::draw(game, target);
    }
};

#endif
