#include "objects.h"
#include "world.h"
#include "framework.h"

// relative sizes
constexpr float GroundRelHeight = 0.1f;
constexpr float ProjectileRelRadius = 0.005f;
constexpr float HunterRelSize = 0.02f;

float ProjectileVelocityFactor = 100.0f;
float GravityAcceleration = 8.0f;

extern World world;

void createProjectile(Objects& obj)
{
    auto world_size = frame::get_world_size();

    obj.projectileRadius = ProjectileRelRadius * world_size.x;
    obj.projectilePosition = { 0.1f * world_size.x, GroundRelHeight * world_size.y + obj.projectileRadius };

    obj.projectile = world.CreateCircle(obj.projectilePosition, obj.projectileRadius);
    world.SetStatic(obj.projectile, false);
    world.SetFill(obj.projectile, frame::col4::RGB(255, 128, 64));
    world.SetBullet(obj.projectile, true);
}

void createHunter(Objects& obj)
{
    auto world_size = frame::get_world_size();

    obj.hunterSize = HunterRelSize * world_size.x;
    obj.hunterPosition = { world_size.x - 0.1f * world_size.x, world_size.y - 0.1f * world_size.x };

    obj.hunter = world.CreateRectangle(obj.hunterPosition, obj.hunterSize, obj.hunterSize);
    world.SetStatic(obj.hunter, false);
    world.SetFill(obj.hunter, frame::col4::RGB(255, 64, 128));
}

void createGround(Objects& obj)
{
    auto world_size = frame::get_world_size();

    obj.ground = world.CreateRectangle(frame::get_world_position_screen_relative({ 0.5f, GroundRelHeight / 2.0f }), world_size.x, GroundRelHeight * world_size.y);
    world.SetFill(obj.ground, frame::col4::RGB(100, 100, 90));
    world.SetLayer(obj.ground, Background);

    obj.groundHeight = GroundRelHeight * world_size.y;
}

Objects createObjects()
{
    Objects result;

    createProjectile(result);
    createHunter(result);
    createGround(result);

    updateProjectileVelocity(result);

    return result;
}

void destroyObjects(Objects& obj)
{
    world.Destroy(obj.projectile);
    world.Destroy(obj.hunter);
    world.Destroy(obj.ground);
}

void updateProjectileVelocity(Objects& obj)
{
    auto p1 = world.GetPosition(obj.projectile);
    auto p2 = world.GetPosition(obj.hunter);

    obj.projectileVelocity = (p2 - p1).normalized() * ProjectileVelocityFactor;

    obj.velocityLength = 40.0f + 20.0f * (ProjectileVelocityFactor - 50.0f) / 150.0f;
}

void updateGravity(Objects& obj)
{
    obj.gravityLength = 40.0f + 20.0f * (GravityAcceleration - 5.0f) / 10.0f;
}
