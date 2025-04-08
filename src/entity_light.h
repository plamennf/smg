#pragma once

#include "entity.h"

struct Light : public Entity {
    float radius;
    Vector3 color;
};
