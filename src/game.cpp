#include "game.h"

#include <SFML/Graphics.hpp>

#include "entity/entity.h"
//#include "entity/entities/player.cpp"
//#include "entity/entities/npc.cpp"
//#include "level/levels/level1.cpp"
#include "SFML/Audio/Music.hpp"

Game::Game() {

    /** 1. Créer la fenetre (d'une dimension de 400 par 400) assigner là à l'objet courant Game (this) puis récupérer la vue par défaut
     * Pour plus d'infos sur les Window et les View :
     * Window : https://www.sfml-dev.org/tutorials/3.1/graphics/draw/#the-drawing-window
     * View : https://www.sfml-dev.org/tutorials/3.1/graphics/view/
     */
}

void Game::initialize() {

    /**
     *  initialize(); est l'entrée du programme, elle est directement appelé par la fonction main() (que vous pouvez voir tout en bas de ce fichier)
     *  juste après que le constructeur (juste au dessus) soit appelé.
     *  C'est ici que son initialisé les levels, entitées de base, musique, etc
     *
     *  Vous pourrez décommenter au fur et à mesure les commentaires de ces lignes afin d'ajouter les éléments (level, player, npc)
     *  au jeu
     */

    //levels.push_back(std::make_unique<Level1>());
    //currentLevel = levels.front().get();

    /**
     * 2. Décommenter le player. Vous allez d'abord créer un fichier entity.cpp qui sera hérité par le joueur (player.cpp) et un PNJ (npc.cpp)
     */

    //sf::Texture playerTexture = sf::Texture(RESOURCES_DIR "/player/player_sprite_sheet.png");
    //auto player = std::make_shared<Player>(playerTexture);
    //player->setPosition({100.f, 100.f});
    //player->setVisible(true);
    //addEntity(player);


    //sf::Texture npcTexture = sf::Texture(RESOURCES_DIR "/player/npc_sprite_sheet.png");
    //auto npc = std::make_shared<NPC>(1, npcTexture);
    //npc->setPosition({250.f, 250.f});
    //npc->setVisible(true);
    //addEntity(npc);

    /** x. Lancer la musique d'ambiance ici
     *  (hint: https://www.sfml-dev.org/tutorials/3.1/audio/sounds/#loading-and-playing-a-sound)
     */


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

    // Dans la Game Loop, vous devez : 1. Mettre à jour les entitées (et en temps normal, mais pas ici, le monde), 2. Faire le rendu
    this->updateThings();

    window.setView(view);
    window.clear();

    this->drawThings();

    window.display();
}

/**
 * Ici, vous mettez à jour TOUTES les entités (entities)
 */
void Game::updateThings() {

    /** Pour faire en sorte que vos entitées et votre physique fonctionnent à la même vitesse, peut importe l'ordinateur et la fréquence de rafraichissement de l'écran,
     * vous avez d'abord besoin du deltaTime.
     * Le deltaTime, c'est le temps passé entre le début de la dernière itération de la loop, et le début de l'itération courante
     * Pour faire cela, SFML propose une classe utilitaire d'horloge : sf::Clock (hint: https://www.sfml-dev.org/tutorials/3.1/system/time/)
     * La méthode d'update des entitées a besoin de ce dT
     */


    // updating...
}

/**
 * Ici, vous dessinez le monde puis TOUTES les entités (attention, pas l'inverse, sinon vous ne verrez pas vos entitées qui seront dessinées derrière la map)
 */
void Game::drawThings() {
    // drawing...
}

bool Game::isFree(const sf::FloatRect &rect, Entity &ent) const {
    // x. Il faut ici renvoyé vrai ou faux selon si une entité (ent) peut ou non aller dans la hitbox (rect) dans le level courant
    // Il faut à la fois voir si l'entité serait en dehors de la boîte puis s'il intéragit avec d'autres entitées
    // (hint : on cherche des intersections entre deux rectangles pour les entités (FloatRect::findIntersection))
    // pour les collisions avec des masks sur le level, il faut regarder du côté de la méthode isCollinding() du level


    return true;
}

/**
 * Le code sous ce commentaire n'a pas forcément besoin d'être compris, vous pouvez essayer de le comprendre pour savoir comment est (simplement) géré le système d'entité mais parce qu'ils utilisent des pointeurs partagés
 */

void Game::onResize(const sf::Vector2u size) {
    view.setSize(sf::Vector2f(size));
}

sf::View& Game::getView() {
    return this->view;
}

void Game::setView(const sf::View &view) {
    this->view = view;
}

void Game::addEntity(std::shared_ptr<Entity> entity) {
    if (!hasEntity(entity)) {
        entities.push_back(std::move(entity));
    }
}

bool Game::hasEntity(std::shared_ptr<Entity> entity) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (*it == entity) {
            return true;
        }
    }
    return false;
}

Entity* Game::getEntity(long int id) {
    for (auto &entity : entities) {
        if (entity->getId() == id) {
            return entity.get();
        }
    }
    return nullptr;
}

bool Game::removeEntity(const Entity *entity) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (entity == it->get()) {
            entities.erase(it);
            return true;
        }
    }
    return false;
}

bool Game::removeEntity(const long int id) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (it->get()->getId() == id) {
            entities.erase(it);
            return true;
        }
    }
    return false;
}

int main() {
    Game game;
    game.initialize();
}