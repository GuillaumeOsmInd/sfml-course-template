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

Game *Game::INSTANCE = nullptr;

Game &Game::getInstance() {
    assert(INSTANCE != nullptr && "Game::getInstance() appele avant la creation du jeu dans main()");
    return *INSTANCE;
}

Game::Game() {
    // Enregistree des le debut : tout ce qui est construit ensuite (niveaux,
    // entites, overlays) peut deja appeler getInstance().
    assert(INSTANCE == nullptr && "Game est un singleton : une seule instance");
    INSTANCE = this;

    this->window = sf::RenderWindow(sf::VideoMode({400, 400}), "Super Mario Bros!");
    this->window.setFramerateLimit(240);
    this->view = window.getDefaultView();

    // Police pixel : sans lissage, le texte agrandi reste net.
    this->font.setSmooth(false);
}

Game::~Game() {
    INSTANCE = nullptr;
}

void Game::initialize() {

    //levels.push_back(std::make_shared<Level0>());
    levels.push_back(std::make_unique<MenuLevel>());
    levels.push_back(std::make_unique<Level1_1>());
    levels.push_back(std::make_unique<Level1_2>());
    //levels.push_back(std::make_unique<Level2>());
    //levels.push_back(std::make_unique<Level3>());

    sf::Image playerImage(RESOURCES_DIR "/textures/mario/mario.png");
    playerImage.createMaskFromColor(sf::Color(0x92, 0x90, 0xFF));
    playerTexture = sf::Texture(playerImage);

    // Overlays communs a tous les niveaux.
    addOverlay(std::make_shared<HUD>(*this));
    addOverlay(std::make_shared<GameOverOverlay>(*this));   // en dernier : au-dessus de tout

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

    if (gameOver) {
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
    }

    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const auto &entity) {
        return entity->hasFallenOutOfLevel() && dynamic_cast<Player*>(entity.get()) == nullptr;
    }), entities.end());
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

const Level *Game::getCurrentLevel() const {
    return this->currentLevel;
}

bool Game::isFree(const sf::FloatRect &from, const sf::FloatRect &to, Entity &ent, const FacingDirection side) const {
    if (currentLevel != nullptr && currentLevel->isCollinding(from, to, side, ent.getZone())) {
        ent.interactWithLevel(side);
        return false;
    }

    for (const auto &entity : this->currentLevel->getEntities()) {
        if (entity.get() == &ent || !entity->isSolid())
            continue;
        if (to.findIntersection(entity->getHitbox())) {
            ent.interactWith(*entity, side);
            entity->interactWith(ent, opposite(side));
            return false;
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

    if (currentLevel != nullptr) {
        currentLevel->unload();
        currentLevel->getEntities().clear();
    }

    currentLevel = next;
    currentLevelIndex = index;
    currentLevel->load(*this);

    auto player = std::make_shared<Player>(playerTexture);
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

    // Verticalement, la camera montre exactement la zone de Mario. Apres une
    // teleportation (tuyau), elle change donc de zone a la frame suivante.
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
    // Simple drapeau : endGame() peut etre appelee pendant la mise a jour des
    // entites (mort de Mario), on ne touche donc a aucune liste ici.
    // Le reste est fait par updateThings() et GameOverOverlay::isVisible().
    gameOver = true;
}

bool Game::isGameOver() const {
    return gameOver;
}

int main() {
    Game game;
    game.initialize();
}
