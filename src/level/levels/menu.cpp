//
// Created by BOUTIGNI on 07-10-26.
//

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