// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_LEVEL_H
#define SFMLPROJECT_LEVEL_H

#include <filesystem>

#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Image.hpp"

class Level {
public:
    explicit Level(const std::filesystem::path &backgroundFile);
    virtual ~Level() = default;

    void load();
    void unload();

    void draw();

    const sf::Sprite& getBackground() const;

    bool isCollinding(const sf::FloatRect &rect) const;
protected:
    // La texture est membre : le sprite ne fait que la referencer.
    sf::Texture backgroundTexture;
    sf::Sprite background;
    sf::Image collisionMask;
};


#endif
