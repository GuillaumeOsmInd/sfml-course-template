// ****************************************************
// Created by Guillaume Boutigny on 10/8/2026.
// ****************************************************

#ifndef SFMLPROJECT_MENU_OVERLAY_H
#define SFMLPROJECT_MENU_OVERLAY_H

#include "../overlay.h"
#include "../../game.h"
#include "../../level/levels/menu.cpp"

class MenuOverlay : public Overlay {
public:
    explicit MenuOverlay(const Game &game) {
        add<OverlayClickArea>(sf::FloatRect({70.f, 124.f}, {100.f, 41.f}), [](Game &g) {
            g.changeLevel(1);   // Level1_1, voir Game::initialize
        });
    }

    bool isVisible(const Game &game) const override {
        return dynamic_cast<const MenuLevel *>(game.getCurrentLevel()) != nullptr;
    }
};

#endif
