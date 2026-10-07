// ****************************************************
// Created by Guillaume Boutigny on 10/7/2026.
// ****************************************************

#ifndef SFMLPROJECT_OVERLAY_H
#define SFMLPROJECT_OVERLAY_H

#include "SFML/Graphics/RenderWindow.hpp"

class Game;

class Overlay {
public:
    bool isVisible() const {
        return true;
    }

    virtual void draw(Game &game, sf::RenderWindow &window);
};


#endif //SFMLPROJECT_OVERLAY_H
