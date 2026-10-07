// ****************************************************
// Created by Guillaume Boutigny on 10/7/2026.
// ****************************************************

#include "overlay.h"

#include <cmath>

#include "SFML/Window/Mouse.hpp"

#include "../game.h"

namespace {
    sf::Vector2f roundVector(const sf::Vector2f v) {
        return {std::round(v.x), std::round(v.y)};
    }

    const sf::Color BUTTON_FILL(0, 0, 0, 170);
    const sf::Color BUTTON_FILL_HOVERED(0, 0, 0, 220);
    const sf::Color BUTTON_TEXT = sf::Color::White;
    const sf::Color BUTTON_TEXT_HOVERED(252, 188, 60);   // jaune des pieces
}

// ---------------------------------------------------------------- OverlayText

OverlayText::OverlayText(const sf::Font &font, const sf::Vector2f position, const sf::String &string,
                         const unsigned characterSize, const Align align)
    : text(font, string, characterSize), align(align) {
    text.setPosition(roundVector(position));
    realign();
}

OverlayText &OverlayText::bindTo(Provider provider) {
    this->provider = std::move(provider);
    return *this;
}

void OverlayText::setString(const sf::String &string) {
    if (text.getString() != string) {
        text.setString(string);
        realign();
    }
}

void OverlayText::setColor(const sf::Color color) {
    text.setFillColor(color);
}

void OverlayText::setPosition(const sf::Vector2f position) {
    text.setPosition(roundVector(position));
}

void OverlayText::update(const Game &game) {
    if (provider) {
        setString(provider(game));
    }
}

void OverlayText::draw(sf::RenderTarget &target) const {
    target.draw(text);
}

void OverlayText::realign() {
    const sf::FloatRect bounds = text.getLocalBounds();
    float x = 0.f;
    switch (align) {
        case Align::LEFT:   x = 0.f; break;
        case Align::CENTER: x = bounds.position.x + bounds.size.x / 2.f; break;
        case Align::RIGHT:  x = bounds.position.x + bounds.size.x; break;
    }
    text.setOrigin(roundVector({x, 0.f}));
}

// -------------------------------------------------------------- OverlayButton

OverlayButton::OverlayButton(const sf::Font &font, const sf::FloatRect bounds, const sf::String &label,
                             Action onClick, const unsigned characterSize)
    : background(bounds.size), label(font, label, characterSize), onClick(std::move(onClick)) {
    background.setPosition(bounds.position);
    background.setOutlineThickness(-1.f);   // vers l'interieur : reste dans `bounds`
    setLabel(label);
    refreshColors();
}

void OverlayButton::setLabel(const sf::String &label) {
    this->label.setString(label);
    const sf::FloatRect text = this->label.getLocalBounds();
    const sf::FloatRect box = background.getGlobalBounds();
    this->label.setOrigin(roundVector(text.position + text.size / 2.f));
    this->label.setPosition(roundVector(box.position + box.size / 2.f));
}

bool OverlayButton::handleEvent(Game &game, const sf::Event &event, const sf::Vector2f mouse) {
    const bool inside = background.getGlobalBounds().contains(mouse);

    if (event.is<sf::Event::MouseMoved>()) {
        if (hovered != inside) {
            hovered = inside;
            refreshColors();
        }
        return false;   // le survol n'empeche pas les autres elements de le voir
    }

    if (const auto *pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (inside && pressed->button == sf::Mouse::Button::Left) {
            // Rien apres l'action : elle peut modifier l'overlay qui nous contient.
            if (onClick) {
                onClick(game);
            }
            return true;
        }
    }
    return false;
}

void OverlayButton::draw(sf::RenderTarget &target) const {
    target.draw(background);
    target.draw(label);
}

void OverlayButton::refreshColors() {
    background.setFillColor(hovered ? BUTTON_FILL_HOVERED : BUTTON_FILL);
    background.setOutlineColor(hovered ? BUTTON_TEXT_HOVERED : BUTTON_TEXT);
    label.setFillColor(hovered ? BUTTON_TEXT_HOVERED : BUTTON_TEXT);
}

// -------------------------------------------------------------------- Overlay

void Overlay::update(const Game &game, const float dt) {
    for (const auto &element : elements) {
        if (element->isVisible()) {
            element->update(game);
        }
    }
}

void Overlay::draw(const Game &game, sf::RenderTarget &target) const {
    for (const auto &element : elements) {
        if (element->isVisible()) {
            element->draw(target);
        }
    }
}

bool Overlay::handleEvent(Game &game, const sf::Event &event, const sf::RenderWindow &window) {
    // Position de la souris au moment de l'evenement si c'en est un, sinon
    // position courante. Puis conversion pixels fenetre -> coordonnees d'overlay.
    sf::Vector2i pixel = sf::Mouse::getPosition(window);
    if (const auto *moved = event.getIf<sf::Event::MouseMoved>()) {
        pixel = moved->position;
    } else if (const auto *pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        pixel = pressed->position;
    } else if (const auto *released = event.getIf<sf::Event::MouseButtonReleased>()) {
        pixel = released->position;
    }
    const sf::Vector2f mouse = window.mapPixelToCoords(pixel, game.getOverlayView());

    // Du dernier au premier : l'element dessine au-dessus a la priorite.
    for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
        if ((*it)->isVisible() && (*it)->handleEvent(game, event, mouse)) {
            return true;   // on s'arrete : `elements` a pu changer pendant l'action
        }
    }
    return false;
}
