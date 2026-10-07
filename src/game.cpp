#include "game.h"

#include <algorithm>

#include <SFML/Graphics.hpp>

#include "entity/entity.h"
#include "level/levels/level1_1.cpp"
#include "SFML/Audio/Music.hpp"
#include "entity/entities/player/player.cpp"
#include "entity/entities/enemies/enemy.cpp"

Game::Game() {
    this->window = sf::RenderWindow(sf::VideoMode({400, 400}), "Super Mario Bros!");
    this->window.setFramerateLimit(240);
    this->view = window.getDefaultView();
}

void Game::initialize() {

    //levels.push_back(std::make_shared<Level0>());
    levels.push_back(std::make_unique<Level1_1>());
    //levels.push_back(std::make_unique<Level2>());
    //levels.push_back(std::make_unique<Level3>());

    // Chargee une seule fois, avant le premier niveau. Le fond des cases de la
    // planche (#9290FF) devient transparent.
    sf::Image playerImage(RESOURCES_DIR "/textures/mario/mario.png");
    playerImage.createMaskFromColor(sf::Color(0x92, 0x90, 0xFF));
    playerTexture = sf::Texture(playerImage);

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
    this->currentLevel->update(dt);
    for (auto &entity : this->currentLevel->getEntities()) {
        entity->update(dt, view, *this);
    }

    // Apres la boucle seulement : changeLevel() et erase() modifient la liste
    // des entites, ce qui est interdit pendant qu'on la parcourt.
    auto &entities = this->currentLevel->getEntities();

    // Mario mort : on attend qu'il sorte du cadre par le bas, puis le niveau
    // repart de zero. Le haut de sa hitbox doit passer sous le bas de la vue.
    if (const auto p = player.lock()) {
        const auto *mario = dynamic_cast<const Player *>(p.get());
        const float frameBottom = view.getCenter().y + view.getSize().y / 2.f;
        if (mario != nullptr && mario->isDead() && mario->getHitbox().position.y >= frameBottom) {
            changeLevel(currentLevelIndex);
            return;
        }
    }

    // Les autres entites tombees hors du niveau (ennemis...) sont retirees.
    // Mario n'est jamais retire : sa chute passe par Player::die(), et le
    // niveau redemarre quand il sort du cadre (ci-dessus).
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

    for (const auto &overlay : this->overlays) {
        if (overlay->isVisible()) {
            overlay->draw(*this, window);
        }
    }
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

    // Au premier appel (depuis initialize), aucun niveau n'est encore charge :
    // currentLevel vaut nullptr, il n'y a rien a quitter.
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
    this->player = player;   // la camera suit sa zone

    // Vue carree d'une zone, quelle que soit la hauteur du fichier de niveau.
    view.setSize({ZONE_HEIGHT, ZONE_HEIGHT});

    // Fenetre carree, au plus grand multiple entier de la zone qui tient a
    // l'ecran : le pixel art reste net. Elle ne depend plus du niveau.
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

int main() {
    Game game;
    game.initialize();
}
