// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#include "level.h"

// Definition complete d'Entity : necessaire pour appeler ses methodes
// (getId...). Dans level.h, la declaration anticipee "class Entity;" suffit.
#include "../entity/entity.h"

Level::Level(const std::filesystem::path &backgroundFile) : backgroundTexture(backgroundFile), background(backgroundTexture), foreground(backgroundTexture) {
    this->collisionMask = backgroundTexture.copyToImage();

    const sf::Image image = backgroundTexture.copyToImage();
    collisionMask = image;

    sf::Image front(image.getSize(), sf::Color::Transparent);
    for (unsigned y = 0; y < image.getSize().y; ++y) {
        for (unsigned x = 0; x < image.getSize().x; ++x) {
            const sf::Color c = image.getPixel({x, y});
            if (c.g & 1) {   // vert impair = premier plan
                front.setPixel({x, y}, c);
            }
        }
    }

    foregroundTexture = sf::Texture(front);
    foreground.setTexture(foregroundTexture, true);
    foreground.setPosition(background.getPosition());
    foreground.setScale(background.getScale());
}

void Level::update(const float dt) {

}

void Level::draw(sf::RenderWindow &window, const RenderPass renderPass) const {
    switch (renderPass) {
        case RenderPass::BEFORE_ENTITIES:
            window.draw(background);
            break;
        case RenderPass::AFTER_ENTITIES:
            window.draw(foreground);
            break;
    }
}

const sf::Sprite& Level::getBackground() const {
    return this->background;
}

const sf::Sprite& Level::getForeground() const {
    return this->foreground;
}

sf::Vector2f Level::getSize() const {
    return this->background.getGlobalBounds().size;
}

sf::Vector2f Level::getSpawnPoint() const {
    return {100.f, 100.f};
}

bool Level::isCollinding(const sf::FloatRect &from, const sf::FloatRect &to, const FacingDirection side) const {
    const sf::Vector2u size = collisionMask.getSize();
    if (size.x == 0) return false;

    /**
     *  Mon système de collision n'utilise pas d'une image de mask à proprement parlé.
     *  J'ai encodé mon mask directement dans l'image du background avec une technique d'encodage avec perte mais "visually lossless" :
     *  L'idée est d'utiliser la composante rouge pour dire si un pixel est un mur ou non :
     *   - Si la composante est paire -> ce n'est pas un mur
     *   - Si elle est impaire -> c'est un mur
     *  Même principe pour le vert (premier plan) et le bleu (collision uniquement par le haut).
     *
     *  Cette technique est quasi impercéptible pour la texture affiché du fait de la très faible différence de couleur.
     *  Un papier sur le principe de steganograpghie et de l'impércéptibilité des différences :
     *  (Jessica Fridrich et al. (2001) — Reliable Detection of LSB Steganography in Color Images (ACM Workshop on Multimedia))
     *
     *  J'ai fait un généré un petit script (/scripts/encode_collision.py) pour créer ces textures spéciales.
     *
     *  (Oui, je me suis embêté pour pas grand chose)
     */
    auto pixel = [&](const int x, const int y) { return collisionMask.getPixel({unsigned(x), unsigned(y)}); };
    auto inside = [&](const int x, const int y) { return x >= 0 && y >= 0 && x < int(size.x) && y < int(size.y); };

    // 1. Murs (rouge impair) et bords du niveau : bloquent dans toutes les directions,
    //    sauf en haut (Mario peut sauter hors de l'ecran) et en bas (il tombe
    //    et meurt : voir fallsOutOfBottom).
    const sf::Vector2f corners[4] = {
        to.position,
        {to.position.x + to.size.x, to.position.y},
        {to.position.x, to.position.y + to.size.y},
        to.position + to.size
    };
    for (const sf::Vector2f &c : corners) {
        // Comparaisons en float : int(-0.5f) vaudrait 0, soit "dans le niveau".
        if (c.x < 0 || c.x >= size.x) return true;    // gauche, droite
        if (c.y < 0 || c.y >= size.y) continue;       // au-dessus ou en dessous : vide, pas de pixel a lire
        if (pixel(int(c.x), int(c.y)).r & 1) return true;
    }

    // 2. Plateformes traversables (bleu impair) : ne bloquent qu'en descendant.
    if (side != DOWN) return false;

    auto isOneWay = [&](const int x, const int y) { return inside(x, y) && (pixel(x, y).b & 1); };

    // Les pieds bloquent s'ils passent par le DESSUS d'une plateforme pendant
    // ce deplacement (pixel traversable sans plateforme juste au-dessus).
    // Des pieds deja a l'interieur, apres un saut par en dessous, ne croisent
    // aucun dessus : l'entite continue de passer a travers.
    const int left = int(to.position.x);
    const int right = int(to.position.x + to.size.x);
    for (int y = int(from.position.y + from.size.y); y <= int(to.position.y + to.size.y); ++y) {
        for (const int x : {left, right}) {
            if (isOneWay(x, y) && !isOneWay(x, y - 1)) return true;
        }
    }
    return false;
}

void Level::addEntity(std::shared_ptr<Entity> entity) {
    if (!hasEntity(entity)) {
        entities.push_back(std::move(entity));
    }
}

bool Level::hasEntity(std::shared_ptr<Entity> entity) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (*it == entity) {
            return true;
        }
    }
    return false;
}

Entity* Level::getEntity(long int id) {
    for (auto &entity : entities) {
        if (entity->getId() == id) {
            return entity.get();
        }
    }
    return nullptr;
}

bool Level::removeEntity(const Entity *entity) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (entity == it->get()) {
            entities.erase(it);
            return true;
        }
    }
    return false;
}

bool Level::removeEntity(const long int id) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (it->get()->getId() == id) {
            entities.erase(it);
            return true;
        }
    }
    return false;
}

std::vector<std::shared_ptr<Entity>> &Level::getEntities() {
    return entities;
}

const std::vector<std::shared_ptr<Entity>> &Level::getEntities() const {
    return entities;
}

bool Level::fallsOutOfBottom(const sf::FloatRect &from, const sf::FloatRect &to) const {
    // On compare le HAUT de la hitbox : l'entite meurt une fois entierement
    // sortie de l'ecran, pas des que ses pieds touchent le bord.
    const float bottom = getSize().y;
    return from.position.y < bottom && to.position.y >= bottom;
}
