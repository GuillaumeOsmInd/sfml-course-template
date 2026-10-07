// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_ZONE_H
#define SFMLPROJECT_ZONE_H

#include <algorithm>
#include <cmath>

constexpr float ZONE_HEIGHT = 240.f;

inline int zoneAt(const float y) {
    return std::max(0, static_cast<int>(std::floor(y / ZONE_HEIGHT)));
}

#endif
