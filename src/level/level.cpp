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

bool Level::isCollinding(const sf::FloatRect &rect) const {
    const sf::Vector2u size = collisionMask.getSize();
    if (size.x == 0) return false;

    const sf::Vector2f corners[4] = {
        rect.position,
        {rect.position.x + rect.size.x, rect.position.y},
        {rect.position.x, rect.position.y + rect.size.y},
        rect.position + rect.size
    };

    for (const sf::Vector2f &c : corners) {
        if (c.x < 0 || c.y < 0 || c.x >= size.x || c.y >= size.y) return true;

        if (collisionMask.getPixel({unsigned(c.x), unsigned(c.y)}).r & 1) return true;   // rouge impair = mur
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
    }
    return false;
}

bool Level::landsOnOneWay(const sf::FloatRect &from, const sf::FloatRect &to) const {
    const sf::Vector2u size = collisionMask.getSize();
    if (size.x == 0) return false;

    const float fromFeet = from.position.y + from.size.y;
    const float toFeet = to.position.y + to.size.y;
    if (toFeet <= fromFeet) return false;   // on monte : on passe toujours a travers

    auto isOneWay = [&](const int x, const int y) {
        if (x < 0 || y < 0 || x >= int(size.x) || y >= int(size.y)) return false;
        return (collisionMask.getPixel({unsigned(x), unsigned(y)}).b & 1) != 0;   // bleu impair
    };

    // Memes points que isCollinding : les deux coins du bas.
    const int left = int(to.position.x);
    const int right = int(to.position.x + to.size.x);

    // Toutes les lignes parcourues par les pieds pendant ce deplacement.
    for (int y = int(fromFeet); y <= int(toFeet); ++y) {
        for (const int x : {left, right}) {
            // Le DESSUS d'une plateforme : pixel traversable sans plateforme
            // juste au-dessus. Des pieds deja a l'interieur (saut par en
            // dessous) ne croisent pas de dessus et continuent de passer.
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
