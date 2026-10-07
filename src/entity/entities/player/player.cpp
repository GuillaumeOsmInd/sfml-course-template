// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#include <algorithm>
#include <iostream>

#include "../humanoid.cpp"
#include "../enemies/enemy.cpp"

class Player : public Humanoid {
public:
    explicit Player(const sf::Texture& texture) : Humanoid(0, texture, sf::Vector2f(0, 0)) {
        sprite.setTextureRect({{0, 8}, {16, 16}});
        sprite.setOrigin({8, 8});
    }

    void update(float dt, sf::View &view, const Game &game) override {
        // Mort : Humanoid gere la chute sans collisions. Pas de controles, et
        // la camera reste figee. Le niveau redemarre quand Mario sort du cadre
        // (voir Game::updateThings).
        if (isDead()) {
            Humanoid::update(dt, view, game);
            return;
        }

        dir.x = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
            dir.x += -1;
            facing_direction_ = LEFT;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            dir.x += 1;
            facing_direction_ = RIGHT;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            jump();
        }

        Humanoid::update(dt, view, game);

        // Mort pendant cette frame (chute dans un trou) : onDeath() a deja
        // choisi le sprite, et la camera reste figee.
        if (isDead()) {
            return;
        }

        if (on_ground) {
            if (dir.x != 0) {
                sprite.setTextureRect({ {20 + (static_cast<int>(animTime / 0.1) % 3) * 18, 8}, {16, 16}});
            } else {
                sprite.setTextureRect({{0, 8}, {16, 16}});
            }
        } else {
            sprite.setTextureRect({ {96, 8}, {16, 16}});
        }

        view.setCenter(this->getPosition() + this->getSprite().getGlobalBounds().size / 2.f);
    }

    void interactWith(Entity &entity, FacingDirection side) const override {
        if (dynamic_cast<Enemy*>(&entity) != nullptr) {
        }
    }

    template<typename Base, typename T>
    inline bool instanceof(const T *ptr) {
        return dynamic_cast<const Base*>(ptr) != nullptr;
    }

protected:
    // Animation de mort de Mario : petit saut vers le haut et sprite de mort.
    // La suite (chute sans collisions) est commune : voir Humanoid::die().
    void onDeath() override {
        velocity.y = -DEATH_JUMP_SPEED;
        sprite.setTextureRect({{116, 8}, {16, 16}});
    }

private:
    static constexpr float DEATH_JUMP_SPEED = 300.f;   // impulsion vers le haut a la mort, pixels/s
};

