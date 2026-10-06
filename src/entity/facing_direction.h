// ****************************************************
// Created by Guillaume Boutigny on 9/21/2026.
// ****************************************************

#ifndef SFMLPROJECT_FACING_DIRECTION_H
#define SFMLPROJECT_FACING_DIRECTION_H

enum FacingDirection {
    DOWN,
    LEFT,
    RIGHT,
    UP
};

constexpr FacingDirection opposite(const FacingDirection direction) {
    switch (direction) {
        case UP:    return DOWN;
        case DOWN:  return UP;
        case LEFT:  return RIGHT;
        case RIGHT: return LEFT;
    }
    return direction;
}

#endif
