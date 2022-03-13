#pragma once

#include "world.h"

// physics "constants"
extern float ProjectileVelocityFactor;
extern float GravityAcceleration;

constexpr World::Layer Background = -5;

struct Objects
{
    World::Object projectile;
    Point projectilePosition;
    float projectileRadius;
    Point projectileVelocity;

    float velocityLength = 50.0f;
    float gravityLength = 50.0f;

    World::Object hunter;
    Point hunterPosition;
    float hunterSize;

    World::Object ground;
    float groundHeight;               // y coordinate of ground
};

Objects createObjects();
void destroyObjects(Objects& obj);

void updateProjectileVelocity(Objects& obj);
void updateGravity(Objects& obj);
