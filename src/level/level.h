// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_LEVEL_H
#define SFMLPROJECT_LEVEL_H

#include <filesystem>
#include <map>
#include <optional>
#include <utility>

#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "../entity/facing_direction.h"
#include "zone.h"

class Entity;
class Game;

enum RenderPass {
    BEFORE_ENTITIES,
    AFTER_ENTITIES
};

class Level {
public:
    explicit Level(const std::filesystem::path &backgroundFile);
    virtual ~Level() = default;

    virtual void load(Game &game);
    virtual void unload() {}

    void update(float dt);
    void draw(sf::RenderWindow &window, RenderPass renderPass) const;

    const sf::Sprite& getBackground() const;
    const sf::Sprite& getForeground() const;

    bool isExitAt(sf::Vector2f point) const;
    virtual bool needHud() const { return true; }

    sf::Vector2f getSize() const;

    // Rectangle de la zone `zone` que la camera peut montrer : toute la
    // largeur de la texture, sauf si setZoneBounds() l'a restreint.
    sf::FloatRect getZoneBounds(int zone) const;

    virtual sf::Vector2f getSpawnPoint() const;

    bool isCollinding(const sf::FloatRect &from, const sf::FloatRect &to, FacingDirection side, int zone) const;

    bool fallsOutOfBottom(const sf::FloatRect &from, const sf::FloatRect &to, int zone) const;

    void addEntity(std::shared_ptr<Entity> entity);

    // Ajoute deux tunnels relies : entrer dans l'un fait sortir par l'autre.
    // A appeler dans load(). Zones et directions : voir Tunnel.
    //   addTunnels({{912.f, 128.f}, {32.f, 16.f}}, DOWN,
    //              {{792.f, 256.f}, {16.f, 16.f}}, std::nullopt);
    void addTunnels(sf::FloatRect areaA, std::optional<FacingDirection> entryA,
                    sf::FloatRect areaB, std::optional<FacingDirection> entryB);
    bool hasEntity(std::shared_ptr<Entity> entity);

    bool removeEntity(const Entity *entity);

    std::vector<std::shared_ptr<Entity>> &getEntities();
    const std::vector<std::shared_ptr<Entity>> &getEntities() const;
protected:
    // Limite horizontalement la camera dans une zone, pour ne pas montrer ce
    // qui entoure la "vraie" zone dans la texture (souterrain de 1-1...).
    // A appeler dans le constructeur du niveau.
    void setZoneBounds(int zone, float left, float right);

    sf::Texture backgroundTexture;
    sf::Sprite background;

    sf::Texture foregroundTexture;
    sf::Sprite foreground;

    sf::Image collisionMask;

    std::map<int, std::pair<float, float>> zoneBounds;   // zone -> (gauche, droite)

    std::vector<std::shared_ptr<Entity>> entities{};
};


#endif
