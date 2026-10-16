//
// Created by BOUTIGNI on 07-10-26.
//

#ifndef SFMLPROJECT_MENU_LEVEL_H
#define SFMLPROJECT_MENU_LEVEL_H

#include "../level.h"

class MenuLevel : public Level
{
public:
    MenuLevel() : Level(RESOURCES_DIR "/textures/levels/menu/menu.png") {};

    bool needHud() const override
    {
        return false;
    }
};

#endif
