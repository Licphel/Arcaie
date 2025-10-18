#pragma once
#include <core/math.h>
#include <core/uuid.h>

namespace arc::world
{

struct P_component_motion
{
    quad aabb;
    vec2 pos;
    vec2 prev_pos;
    vec2 velocity;
    vec2 face;
    double rotation;
};

struct level;

struct entity
{
    uuid uuid;
    level *level;
};

} // namespace arc::world