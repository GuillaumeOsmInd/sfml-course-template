// ****************************************************
// Created by Guillaume Boutigny on 10/8/2026.
// ****************************************************

#ifndef SFMLPROJECT_ITEMS_TEXTURE_H
#define SFMLPROJECT_ITEMS_TEXTURE_H

#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"

inline const sf::Texture &itemsTexture() {
    static const sf::Texture texture = [] {
        sf::Image image(RESOURCES_DIR "/textures/items/items.png");
        image.createMaskFromColor(sf::Color(0x92, 0x90, 0xFF));
        return sf::Texture(image);
    }();
    return texture;
}

#endif
