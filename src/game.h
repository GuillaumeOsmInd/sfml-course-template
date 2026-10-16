// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_GAME_H
#define SFMLPROJECT_GAME_H
#include <memory>
#include <vector>

#include "entity/entity.h"
#include "level/level.h"
#include "overlay/overlay.h"
#include "SFML/Graphics/Font.hpp"

class Game {
public:
    static Game &getInstance();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    ~Game();

    void initialize();

    void updateThings();
    void drawThings();

    void loop();

    void onResize(sf::Vector2u size);

    const Level *getCurrentLevel() const;
    void changeLevel(std::size_t index);

    sf::View& getView();
    void setView(const sf::View &view);

    bool isFree(const sf::FloatRect &from, const sf::FloatRect &to, Entity &ent, FacingDirection side) const;

    void spawn(std::shared_ptr<Entity> entity);

    void addOverlay(std::shared_ptr<Overlay> overlay);
    sf::View getOverlayView() const;

    int getLife() const;

    int getCoins() const;
    void addCoin();
    void decrementLife();

    void endGame();
    bool isGameOver() const;

    const sf::Font &getFont() const;

private:
    Game();
    friend int main();

    void updateViewport();
    void clampView();

    bool dispatchToOverlays(const sf::Event &event);

    sf::RenderWindow window;
    sf::View view;

    sf::Clock clock;

    Level* currentLevel = nullptr;

    std::weak_ptr<Entity> player;
    std::size_t currentLevelIndex = 0;
    sf::Texture playerTexture;
    int life = 3;
    int coins = 0;   // conserve d'un niveau a l'autre et apres une mort
    static constexpr int COINS_PER_LIFE = 10;   // pieces pour gagner une vie
    bool gameOver = false;

    std::vector<std::unique_ptr<Level>> levels{};
    std::vector<std::shared_ptr<Entity>> entities{};
    std::vector<std::shared_ptr<Entity>> pendingSpawns{};   // voir spawn()
    std::vector<std::shared_ptr<Overlay>> overlays{};

    sf::Font font = sf::Font(RESOURCES_DIR "/fonts/smb.ttf");

    static Game *INSTANCE;
};

#endif
