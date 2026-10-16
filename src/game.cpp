#include "game.h"

#include <algorithm>
#include <cassert>

#include <SFML/Graphics.hpp>

#include "entity/entity.h"
#include "level/levels/level1.cpp"
#include "level/levels/menu.cpp"
#include "SFML/Audio/Music.hpp"
#include "entity/entities/player/player.cpp"
#include "entity/entities/enemies/enemy.cpp"
#include "overlay/overlays/hud.cpp"
#include "overlay/overlays/game_over.cpp"
#include "overlay/overlays/menu.cpp"

Game *Game::INSTANCE = nullptr;

Game &Game::getInstance() {
    assert(INSTANCE != nullptr && "Game::getInstance() appele avant la creation du jeu dans main()");
    return *INSTANCE;
}

Game::Game() {
    INSTANCE = this;

    this->window = sf::RenderWindow(sf::VideoMode({400, 400}), "Super Mario Bros!");
    this->window.setFramerateLimit(240);
    this->view = window.getDefaultView();

    this->font.setSmooth(false);
}

Game::~Game() {
    INSTANCE = nullptr;
}

void Game::initialize() {

    //levels.push_back(std::make_shared<Level0>());
    levels.push_back(std::make_unique<MenuLevel>());
    levels.push_back(std::make_unique<Level1_2>());
    levels.push_back(std::make_unique<Level1_1>());
    //levels.push_back(std::make_unique<Level2>());
    //levels.push_back(std::make_unique<Level3>());

    sf::Image playerImage(RESOURCES_DIR "/textures/mario/mario.png");
    playerImage.createMaskFromColor(sf::Color(0x92, 0x90, 0xFF));
    playerTexture = sf::Texture(playerImage);

    addOverlay(std::make_shared<HUD>(*this));
    addOverlay(std::make_shared<MenuOverlay>(*this));
    addOverlay(std::make_shared<GameOverOverlay>(*this));

    changeLevel(0);

    while (window.isOpen()) {
        loop();
    }
}

void Game::loop() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
            onResize(resized->size);
        } else {
            dispatchToOverlays(*event);
        }
    }

    this->updateThings();
    this->clampView();

    window.setView(view);
    window.clear();

    this->drawThings();

    window.display();
}

void Game::updateThings() {

    const float dt = std::min(clock.restart().asSeconds(), 0.05f);

    for (const auto &overlay : this->overlays) {
        if (overlay->isVisible(*this)) {
            overlay->update(*this, dt);
        }
    }

    if (isGameOver()) {
        return;
    }

    this->currentLevel->update(dt);
    for (auto &entity : this->currentLevel->getEntities()) {
        entity->update(dt, view, *this);
    }

    auto &entities = this->currentLevel->getEntities();

    if (const auto p = player.lock()) {
        const auto *mario = dynamic_cast<const Player *>(p.get());
        const float frameBottom = view.getCenter().y + view.getSize().y / 2.f;
        if (mario != nullptr && mario->isDead() && mario->getHitbox().position.y >= frameBottom) {
            changeLevel(currentLevelIndex);
            return;
        }

        if (mario != nullptr && !mario->isDead() && currentLevel->isExitAt(mario->getHitbox().getCenter())) {
            changeLevel((currentLevelIndex + 1) % levels.size());
            return;
        }
    }

    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const auto &entity) {
        return (entity->hasFallenOutOfLevel() && dynamic_cast<Player*>(entity.get()) == nullptr)
            || entity->isMarkedForRemoval();
    }), entities.end());

    for (auto &entity : pendingSpawns) {
        currentLevel->addEntity(std::move(entity));
    }
    pendingSpawns.clear();
}

void Game::spawn(std::shared_ptr<Entity> entity) {
    if (entity != nullptr) {
        pendingSpawns.push_back(std::move(entity));
    }
}

void Game::drawThings() {
    this->currentLevel->draw(window, RenderPass::BEFORE_ENTITIES);

    for (const auto &entity : this->currentLevel->getEntities()) {
        if (entity->isVisible()) {
            entity->draw(window, sf::RenderStates::Default);
        }
    }

    this->currentLevel->draw(window, RenderPass::AFTER_ENTITIES);

    window.setView(getOverlayView());
    for (const auto &overlay : this->overlays) {
        if (overlay->isVisible(*this)) {
            overlay->draw(*this, window);
        }
    }
    window.setView(view);
}

bool Game::dispatchToOverlays(const sf::Event &event) {
    const auto targets = this->overlays;
    for (auto it = targets.rbegin(); it != targets.rend(); ++it) {
        if ((*it)->isVisible(*this) && (*it)->handleEvent(*this, event, window)) {
            return true;
        }
    }
    return false;
}

void Game::addOverlay(std::shared_ptr<Overlay> overlay) {
    this->overlays.push_back(std::move(overlay));
}

sf::View Game::getOverlayView() const {
    sf::View overlayView(sf::FloatRect({0.f, 0.f}, {ZONE_HEIGHT, ZONE_HEIGHT}));
    overlayView.setViewport(view.getViewport());
    return overlayView;
}

const sf::Font &Game::getFont() const {
    return this->font;
}

int Game::getLife() const {
    return this->life;
}

int Game::getCoins() const {
    return this->coins;
}

void Game::addCoin() {
    this->coins++;
    if (this->coins >= COINS_PER_LIFE) {
        this->coins -= COINS_PER_LIFE;
        this->life++;
    }
}

const Level *Game::getCurrentLevel() const {
    return this->currentLevel;
}

bool Game::isFree(const sf::FloatRect &from, const sf::FloatRect &to, Entity &ent, const FacingDirection side) const {
    if (currentLevel != nullptr && currentLevel->isCollinding(from, to, side, ent.getZone())) {
        ent.interactWithLevel(side);
        return false;
    }

    for (const auto &entity : this->currentLevel->getEntities()) {
        if (entity.get() == &ent)
            continue;
        if (to.findIntersection(entity->getHitbox())) {
            ent.interactWith(*entity, side);
            entity->interactWith(ent, opposite(side));
            // Une entite non solide (piece posee, Mario mort...) est prevenue du
            // contact mais ne bloque pas : on continue de chercher un obstacle.
            if (entity->isSolid()) {
                return false;
            }
        }
    }

    if (currentLevel != nullptr && side == DOWN && currentLevel->fallsOutOfBottom(from, to, ent.getZone())) {
        ent.markFallenOutOfLevel();
    }
    return true;
}


void Game::onResize(const sf::Vector2u size) {
    updateViewport();
}

void Game::changeLevel(const std::size_t index) {
    Level *next = levels.at(index).get();

    pendingSpawns.clear();

    // Attributs de Mario a transmettre au nouveau niveau, lus AVANT de vider le
    // niveau (qui detruit l'ancien Mario). Apres une mort, Mario est deja
    // redevenu petit : il recommence donc de zero.
    Player::State marioState;
    if (const auto oldMario = std::dynamic_pointer_cast<Player>(player.lock())) {
        marioState = oldMario->getState();
    }

    if (currentLevel != nullptr) {
        currentLevel->unload();
        currentLevel->getEntities().clear();
    }

    currentLevel = next;
    currentLevelIndex = index;
    currentLevel->load(*this);

    auto player = std::make_shared<Player>(playerTexture);
    player->setState(marioState);                    // avant setPosition : la taille compte pour la zone
    player->setPosition(currentLevel->getSpawnPoint());
    player->setVisible(true);
    this->currentLevel->addEntity(player);
    this->player = player;

    view.setSize({ZONE_HEIGHT, ZONE_HEIGHT});

    const unsigned zoneSide = static_cast<unsigned>(ZONE_HEIGHT);
    const unsigned maxSide = sf::VideoMode::getDesktopMode().size.y * 85 / 100;
    const unsigned windowSide = zoneSide * std::max(1u, maxSide / zoneSide);

    if (window.getSize() != sf::Vector2u(windowSide, windowSide)) {
        window.setSize({windowSide, windowSide});
    }
    updateViewport();

    clock.restart();
}

void Game::updateViewport() {
    const sf::Vector2f size(window.getSize());
    sf::FloatRect viewport({0.f, 0.f}, {1.f, 1.f});

    if (size.x > size.y) {
        viewport.size.x = size.y / size.x;
        viewport.position.x = (1.f - viewport.size.x) / 2.f;
    } else if (size.y > size.x) {
        viewport.size.y = size.x / size.y;
        viewport.position.y = (1.f - viewport.size.y) / 2.f;
    }

    view.setViewport(viewport);
}

void Game::clampView() {
    const sf::Vector2f half = view.getSize() / 2.f;
    const sf::Vector2f level = currentLevel->getSize();

    sf::Vector2f center = view.getCenter();
    center.x = std::clamp(center.x, half.x, std::max(half.x, level.x - half.x));

    if (const auto p = player.lock()) {
        center.y = p->getZone() * ZONE_HEIGHT + ZONE_HEIGHT / 2.f;
    }
    view.setCenter(center);
}

sf::View &Game::getView() {
    return this->view;
}

void Game::setView(const sf::View &view) {
    this->view = view;
}

void Game::decrementLife() {
    life--;
    if (life < 0)
    {
        endGame();
    }
}

void Game::endGame() {
    gameOver = true;
}

bool Game::isGameOver() const {
    return gameOver;
}

int main() {
    Game game;
    game.initialize();
}
