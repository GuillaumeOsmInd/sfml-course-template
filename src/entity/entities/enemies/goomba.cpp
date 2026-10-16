// ****************************************************
// Created by Guillaume Boutigny on 10/8/2026.
// ****************************************************

#ifndef SFMLPROJECT_GOOMBA_H
#define SFMLPROJECT_GOOMBA_H

#include "SFML/Graphics/Image.hpp"

#include "enemy.cpp"

class Goomba : public Enemy {
public:
    explicit Goomba(const sf::Vector2f pos) : Enemy(texture()) {
        sprite.setTextureRect(WALK[0]);
        sprite.setOrigin({8, 8});                 // demi-tour sur place
        setPosition(pos + sf::Vector2f(8.f, 8.f));
        setVisible(true);
        walk_speed = 40.f;                        // bien plus lent que Mario
    }

    void update(const float dt, sf::View &view, const Game &game) override {
        if (squashed) {
            Entity::update(dt, view, game);       // juste le temps : il ne bouge plus
            squashTime += dt;
            if (squashTime >= SQUASH_DURATION) {
                markForRemoval();
            }
            return;
        }

        Enemy::update(dt, view, game);
        if (!isDead()) {
            sprite.setTextureRect(WALK[static_cast<int>(animTime / 0.15f) % 2]);
        }
    }

    void stomp() override {
        if (squashed) return;

        // Recompense : une piece, comptee tout de suite. La piece qui jaillit
        // au-dessus du Goomba est purement visuelle (CoinPop) : non solide et
        // ignoree par Player, elle ne gene pas le rebond de Mario et ne peut
        // pas etre ramassee une seconde fois. Ajout differe : on est en pleine
        // boucle sur les entites. (Origine au centre : la case au-dessus du
        // Goomba commence en position - (8, 8 + 16).)
        Game::getInstance().addCoin();
        Game::getInstance().spawn(std::make_shared<CoinPop>(getPosition() - sf::Vector2f(8.f, 24.f)));

        squashed = true;
        dir.x = 0;
        velocity = {0.f, 0.f};
        sprite.setTextureRect(SQUASHED);
        sprite.setOrigin({8, 0});                 // 8 px de haut : pose sur le sol
    }

    bool isSolid() const override { return !squashed && Enemy::isSolid(); }

private:
    static constexpr float SQUASH_DURATION = 0.5f;   // secondes avant de disparaitre
    static constexpr sf::IntRect WALK[2] = {{{0, 16}, {16, 16}}, {{18, 16}, {16, 16}}};
    static constexpr sf::IntRect SQUASHED{{36, 24}, {16, 8}};

    static const sf::Texture &texture() {
        static const sf::Texture tex = [] {
            sf::Image image(RESOURCES_DIR "/textures/ennemies/ennemies.png");
            image.createMaskFromColor(sf::Color(0x92, 0x90, 0xFF));
            return sf::Texture(image);
        }();
        return tex;
    }

    bool squashed = false;
    float squashTime = 0.f;
};

#endif
