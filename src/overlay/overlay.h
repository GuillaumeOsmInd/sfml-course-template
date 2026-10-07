// ****************************************************
// Created by Guillaume Boutigny on 10/7/2026.
// ****************************************************

#ifndef SFMLPROJECT_OVERLAY_H
#define SFMLPROJECT_OVERLAY_H

#include <functional>
#include <memory>
#include <vector>

#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Window/Event.hpp"

class Game;

/**
 *  Systeme d'overlay : des elements (textes, boutons...) dessines par dessus le
 *  niveau, apres le premier plan.
 *
 *  Les coordonnees d'un overlay sont celles de l'ecran du jeu : un carre de
 *  ZONE_HEIGHT x ZONE_HEIGHT pixels, (0, 0) en haut a gauche. Elles ne bougent
 *  pas avec la camera et suivent le redimensionnement de la fenetre
 *  (voir Game::getOverlayView).
 */

class OverlayElement {
public:
    virtual ~OverlayElement() = default;

    virtual void update(const Game &game) {}

    virtual void draw(sf::RenderTarget &target) const = 0;

    virtual bool handleEvent(Game &game, const sf::Event &event, sf::Vector2f mouse) { return false; }

    bool isVisible() const { return visible; }
    void setVisible(const bool visible) { this->visible = visible; }

protected:
    bool visible = true;
};

enum class Align {
    LEFT,
    CENTER,
    RIGHT
};

class OverlayText : public OverlayElement {
public:
    using Provider = std::function<sf::String(const Game &)>;

    OverlayText(const sf::Font &font, sf::Vector2f position, const sf::String &string = "",
                unsigned characterSize = 8, Align align = Align::LEFT);

    OverlayText &bindTo(Provider provider);

    void setString(const sf::String &string);
    void setColor(sf::Color color);
    void setPosition(sf::Vector2f position);

    void update(const Game &game) override;
    void draw(sf::RenderTarget &target) const override;

private:
    void realign();

    sf::Text text;
    Align align;
    Provider provider;
};

class OverlayButton : public OverlayElement {
public:
    using Action = std::function<void(Game &)>;

    OverlayButton(const sf::Font &font, sf::FloatRect bounds, const sf::String &label, Action onClick,
                  unsigned characterSize = 8);

    void setLabel(const sf::String &label);

    bool handleEvent(Game &game, const sf::Event &event, sf::Vector2f mouse) override;
    void draw(sf::RenderTarget &target) const override;

private:
    void refreshColors();

    sf::RectangleShape background;
    sf::Text label;
    Action onClick;
    bool hovered = false;
};

class Overlay {
public:
    virtual ~Overlay() = default;

    virtual bool isVisible(const Game &game) const { return true; }

    virtual void update(const Game &game, float dt);

    virtual void draw(const Game &game, sf::RenderTarget &target) const;

    virtual bool handleEvent(Game &game, const sf::Event &event, const sf::RenderWindow &window);

    template<typename T, typename... Args>
    T &add(Args &&... args) {
        auto element = std::make_unique<T>(std::forward<Args>(args)...);
        T &ref = *element;
        elements.push_back(std::move(element));
        return ref;
    }

protected:
    std::vector<std::unique_ptr<OverlayElement>> elements;
};

#endif //SFMLPROJECT_OVERLAY_H
