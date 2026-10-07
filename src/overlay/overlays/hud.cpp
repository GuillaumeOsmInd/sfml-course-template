// ****************************************************
// Created by Guillaume Boutigny on 10/7/2026.
// ****************************************************

#include "../overlay.h"
#include "../../game.h"

class HUD : public Overlay {
    void draw(Game &game, sf::RenderWindow &window) override {
        sf::View view = game.getView();
        sf::Vector2f center = view.getCenter();
        sf::Vector2f size = view.getSize();


    }
};
