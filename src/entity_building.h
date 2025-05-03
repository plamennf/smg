#pragma once

#include "entity.h"

enum Building_Type {
    BUILDING_TYPE_HOUSE,
    BUILDING_TYPE_TOURNAMENT,
    BUILDING_TYPE_SHOP,
};

struct Building : public Entity {
    Building_Type type;
};
