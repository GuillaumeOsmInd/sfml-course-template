// ****************************************************
// Created by Guillaume Boutigny on 10/6/2026.
// ****************************************************

#ifndef SFMLPROJECT_BLOCK_H
#define SFMLPROJECT_BLOCK_H

#include <functional>
#include <memory>

#include "SFML/Graphics/Image.hpp"

#include "../../entity.h"
#include "../humanoid.cpp"
#include "../items/coin.cpp"
#include "../../../game.h"

class Block : public Entity {
public:
    Block(const sf::Vector2f position, const sf::IntRect textureRect) : Entity(texture(), position) {
        sprite.setTextureRect(textureRect);
        setPosition(position);   // recalcule la zone avec la taille finale du sprite
        setVisible(true);
    }

    void interactWith(Entity &entity, const FacingDirection side) override {
        if (side == DOWN && dynamic_cast<Humanoid *>(&entity) != nullptr) {
            onHitFromBelow();
        }
    }

protected:
    virtual void onHitFromBelow() {}

    static const sf::Texture &texture() {
        static const sf::Texture tex = [] {
            sf::Image image(RESOURCES_DIR "/textures/blocks/blocks.png");
            image.createMaskFromColor(sf::Color(148, 148, 255));
            return sf::Texture(image);
        }();
        return tex;
    }
};

class Brick : public Block {
public:
    explicit Brick(const sf::Vector2f position) : Block(position, {{17, 16}, {16, 16}}) {}

protected:
    void onHitFromBelow() override {
        markForRemoval();
    }
};

class MysteryBlock : public Block {
public:
    using Content = std::function<std::shared_ptr<Entity>(sf::Vector2f)>;

    MysteryBlock(const sf::Vector2f position, Content content)
        : Block(position, FRAMES[0]), content(std::move(content)) {}

    void update(const float dt, sf::View &view, const Game &game) override {
        Entity::update(dt, view, game);
        if (!used) {
            constexpr int CYCLE[4] = {0, 1, 2, 1};
            sprite.setTextureRect(FRAMES[CYCLE[static_cast<int>(animTime / 0.25f) % 4]]);
        }
    }

protected:
    void onHitFromBelow() override {
        if (used) return;
        used = true;
        sprite.setTextureRect(USED);
        if (content) {
            const sf::FloatRect box = getHitbox();
            Game::getInstance().spawn(content({box.position.x, box.position.y - box.size.y}));
        }
    }

private:
    static constexpr sf::IntRect FRAMES[3] = {{{298, 78}, {16, 16}}, {{315, 78}, {16, 16}}, {{332, 78}, {16, 16}}};
    static constexpr sf::IntRect USED{{349, 78}, {16, 16}};

    Content content;
    bool used = false;
};

class CoinBlock : public MysteryBlock {
public:
    explicit CoinBlock(const sf::Vector2f position)
        : MysteryBlock(position, [](const sf::Vector2f cell) {
              Game::getInstance().addCoin();
              return std::make_shared<CoinPop>(cell);
          }) {}
};

class Coin : public Block {
public:
    explicit Coin(const sf::Vector2f position) : Block(position, FRAMES[0]) {}

    bool isSolid() const override { return false; }

    void interactWith(Entity &entity, FacingDirection side) override {}

    void update(const float dt, sf::View &view, const Game &game) override {
        Entity::update(dt, view, game);
        constexpr int CYCLE[4] = {0, 1, 2, 1};
        sprite.setTextureRect(FRAMES[CYCLE[static_cast<int>(animTime / 0.15f) % 4]]);
    }

    void collect() {
        if (collected) return;
        collected = true;
        Game::getInstance().addCoin();
        markForRemoval();
    }

private:
    static constexpr sf::IntRect FRAMES[3] = {{{298, 95}, {16, 16}}, {{315, 95}, {16, 16}}, {{332, 95}, {16, 16}}};

    bool collected = false;
};

#endif
