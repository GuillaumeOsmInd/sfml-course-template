// ****************************************************
// Created by Guillaume Boutigny on 10/16/2026.
// ****************************************************

#ifndef SFMLPROJECT_TUNNEL_H
#define SFMLPROJECT_TUNNEL_H

#include <memory>
#include <optional>

#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Window/Keyboard.hpp"

#include "../../entity.h"

class Tunnel : public Entity {
public:
    static constexpr bool SHOW_AREAS = false;

    Tunnel(const sf::FloatRect area, const std::optional<FacingDirection> entry)
        : Entity(emptyTexture(), area.position), area(area), entry(entry) {
        setPosition(area.position);
        setVisible(SHOW_AREAS);
    }

    static void link(const std::shared_ptr<Tunnel> &a, const std::shared_ptr<Tunnel> &b) {
        a->exit = b;
        b->exit = a;
    }

    sf::FloatRect getHitbox() const override { return area; }

    bool isSolid() const override { return false; }

    std::optional<FacingDirection> getEntry() const { return entry; }
    std::shared_ptr<Tunnel> getExit() const { return exit.lock(); }

    static sf::Keyboard::Key keyFor(const FacingDirection direction) {
        switch (direction) {
            case DOWN:  return sf::Keyboard::Key::S;
            case LEFT:  return sf::Keyboard::Key::Q;
            case RIGHT: return sf::Keyboard::Key::D;
            case UP:    return sf::Keyboard::Key::Z;
        }
        return sf::Keyboard::Key::Unknown;
    }

    bool canEnter(const sf::FloatRect &hitbox, const bool onGround) const {
        if (!entry || !getExit() || !hitbox.findIntersection(area)) {
            return false;
        }
        switch (*entry) {
            case DOWN: {
                const float center = hitbox.position.x + hitbox.size.x / 2.f;
                return onGround && center >= area.position.x && center <= area.position.x + area.size.x;
            }
            case LEFT:
            case RIGHT:
                return onGround;
            case UP:
                return true;
        }
        return false;
    }

    sf::Vector2f exitFeet() const {
        return {area.position.x + area.size.x / 2.f, area.position.y + area.size.y};
    }

    void draw(sf::RenderTarget &target, const sf::RenderStates &states) const override {
        sf::RectangleShape outline(area.size);
        outline.setPosition(area.position);
        outline.setFillColor(sf::Color::Transparent);
        outline.setOutlineColor(entry ? sf::Color::Yellow : sf::Color::Cyan);
        outline.setOutlineThickness(-1.f);
        target.draw(outline, states);
    }

private:
    static const sf::Texture &emptyTexture() {
        static const sf::Texture texture;
        return texture;
    }

    sf::FloatRect area;
    std::optional<FacingDirection> entry;
    std::weak_ptr<Tunnel> exit;
};

#endif
