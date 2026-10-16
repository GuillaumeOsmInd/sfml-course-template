// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#include "../humanoid.cpp"
#include "../enemies/enemy.cpp"
#include "../blocks/block.cpp"
#include "../items/mushroom.cpp"
#include "../tunnels/tunnel.cpp"
#include "../../../game.h"

class Player : public Humanoid {
public:
    explicit Player(const sf::Texture &texture) : Humanoid(texture, sf::Vector2f(0, 0)) {
        sprite.setTextureRect({{0, 8}, {16, 16}});
        sprite.setOrigin({8, 8});
    }

    void update(float dt, sf::View &view, const Game &game) override {
        if (isDead()) {
            Humanoid::update(dt, view, game);
            return;
        }

        // Invincible apres avoir ete touche en grand Mario : il clignote.
        invincibleTime = std::max(0.f, invincibleTime - dt);
        const bool blinkOff = invincibleTime > 0.f && static_cast<int>(invincibleTime * 20.f) % 2 == 0;
        sprite.setColor(sf::Color(255, 255, 255, blinkOff ? 60 : 255));

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

        if (isDead()) {
            return;
        }

        useTunnel(game);

        // Petit et grand Mario ont les memes x sur la planche : seules la ligne
        // (y) et la hauteur changent.
        const int row = big ? 32 : 8;
        const int height = big ? 32 : 16;
        if (on_ground) {
            if (dir.x != 0) {
                sprite.setTextureRect({{20 + (static_cast<int>(animTime / 0.1) % 3) * 18, row}, {16, height}});
            } else {
                sprite.setTextureRect({{0, row}, {16, height}});
            }
        } else {
            sprite.setTextureRect({{96, row}, {16, height}});
        }
        // La camera suit Mario : voir Game::updateCamera.
    }

    void interactWith(Entity &entity, FacingDirection side) override {
        if (auto *coin = dynamic_cast<Coin *>(&entity)) {
            coin->collect();
            return;
        }

        if (auto *mushroom = dynamic_cast<Mushroom *>(&entity)) {
            mushroom->markForRemoval();
            grow();
            return;
        }

        if (auto *enemy = dynamic_cast<Enemy *>(&entity)) {
            if (isDead() || enemy->isDead() || !enemy->isSolid()) {
                return;
            }

            if (side == DOWN) {
                enemy->stomp();
                velocity.y = -STOMP_BOUNCE_SPEED;
            } else if (invincibleTime <= 0.f) {
                hurt();
            }
        }
    }

    // Champignon : le petit Mario devient grand. Sans effet s'il l'est deja.
    void grow() {
        setBig(true);
    }

    // Touche par un ennemi : le grand Mario redevient petit et devient
    // invincible un instant (sinon il mourrait a la frame suivante) ; le
    // petit Mario meurt.
    void hurt() {
        if (!big) {
            die();
            return;
        }
        setBig(false);
        invincibleTime = INVINCIBLE_DURATION;
    }

    bool isBig() const { return big; }

    // Attributs de Mario conserves d'un niveau a l'autre (voir Game::changeLevel).
    // Une mort les remet a zero : Mario mort est toujours redevenu petit.
    struct State {
        bool big = false;
    };

    State getState() const { return {big}; }
    void setState(const State &state) { setBig(state.big); }

    // Invincible, Mario n'est plus solide : les ennemis le traversent au lieu
    // de rester colles contre lui (ils le tueraient a la fin du clignotement).
    bool isSolid() const override { return Humanoid::isSolid() && invincibleTime <= 0.f; }

    template<typename Base, typename T>
    inline bool instanceof(const T *ptr) {
        return dynamic_cast<const Base *>(ptr) != nullptr;
    }

protected:
    void onDeath() override {
        setBig(false);                                // l'animation de mort est celle du petit Mario
        sprite.setColor(sf::Color::White);
        velocity.y = -DEATH_JUMP_SPEED;
        sprite.setTextureRect({{116, 8}, {16, 16}});
        Game::getInstance().decrementLife();
    }

private:
    // Devant un tunnel, avec sa touche enfoncee : Mario ressort par l'autre
    // tunnel de la paire. Apres un passage, les tunnels sont verrouilles tant
    // que Mario reste dans une zone de tunnel en tenant une touche ZQSD : sinon,
    // en gardant S enfonce, il repartirait aussitot par le tunnel de sortie.
    void useTunnel(const Game &game) {
        const sf::FloatRect box = getHitbox();
        bool inTunnelArea = false;

        // Lecture seule de la liste des entites : on est dans la boucle d'update.
        for (const auto &entity : game.getCurrentLevel()->getEntities()) {
            const auto *tunnel = dynamic_cast<const Tunnel *>(entity.get());
            if (tunnel == nullptr || !box.findIntersection(tunnel->getHitbox())) {
                continue;
            }
            inTunnelArea = true;

            if (!tunnelLocked && tunnel->canEnter(box, on_ground)
                && sf::Keyboard::isKeyPressed(Tunnel::keyFor(*tunnel->getEntry()))) {
                exitThrough(*tunnel->getExit());
                tunnelLocked = true;
                return;
            }
        }

        const bool tunnelKeyHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)
                                   || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)
                                   || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)
                                   || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
        if (!inTunnelArea || !tunnelKeyHeld) {
            tunnelLocked = false;
        }
    }

    // Place les pieds de Mario au point de sortie du tunnel, sans elan. En deux
    // temps : ou sont les pieds par rapport a la position depend de sa taille.
    // setPosition (et non sprite.setPosition) : la zone change, la camera suit.
    void exitThrough(const Tunnel &tunnel) {
        velocity = {0.f, 0.f};
        const sf::Vector2f feet = tunnel.exitFeet();
        setPosition(feet);
        const sf::FloatRect box = getHitbox();
        setPosition(getPosition() + sf::Vector2f(feet.x - (box.position.x + box.size.x / 2.f),
                                                 feet.y - (box.position.y + box.size.y)));
    }

    bool tunnelLocked = false;

    // Change de taille. L'origine reste au milieu des pieds a la meme
    // hauteur ((8,8) en 16x16, (8,24) en 16x32) : Mario grandit vers le haut,
    // sans s'enfoncer dans le sol. La hitbox suit le sprite.
    void setBig(const bool value) {
        big = value;
        sprite.setTextureRect(big ? sf::IntRect({0, 32}, {16, 32}) : sf::IntRect({0, 8}, {16, 16}));
        sprite.setOrigin(big ? sf::Vector2f(8.f, 24.f) : sf::Vector2f(8.f, 8.f));
    }

    static constexpr float INVINCIBLE_DURATION = 1.5f;  // secondes apres avoir rapetisse
    bool big = false;
    float invincibleTime = 0.f;

    static constexpr float STOMP_BOUNCE_SPEED = 250.f; // rebond apres avoir ecrase un ennemi, pixels/s
    static constexpr float DEATH_JUMP_SPEED = 300.f; // impulsion vers le haut a la mort, pixels/s
};
