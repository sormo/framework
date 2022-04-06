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
    obj.projectileRadius = ProjectileRelRadius * frame::canvas_width();
    obj.projectilePosition = { 0.1f * frame::canvas_width(), GroundRelHeight * frame::canvas_height() + obj.projectileRadius };

    obj.projectile = world.CreateCircle(obj.projectilePosition, obj.projectileRadius);
    world.SetStatic(obj.projectile, false);
    world.SetFill(obj.projectile, Color::RGB(255, 128, 64));
    world.SetBullet(obj.projectile, true);
}

void createHunter(Objects& obj)
{
    obj.hunterSize = HunterRelSize * frame::canvas_width();
    obj.hunterPosition = { frame::canvas_width() - 0.1f * frame::canvas_width(), frame::canvas_height() - 0.1f * frame::canvas_width() };

    obj.hunter = world.CreateRectangle(obj.hunterPosition, obj.hunterSize, obj.hunterSize);
    world.SetStatic(obj.hunter, false);
    world.SetFill(obj.hunter, Color::RGB(255, 64, 128));
}

void createGround(Objects& obj)
{
    obj.ground = world.CreateRectangle(frame::rel_pos(0.5f, GroundRelHeight / 2.0f), frame::canvas_width(), GroundRelHeight * frame::canvas_height());
    world.SetFill(obj.ground, Color::RGB(100, 100, 90));
    world.SetLayer(obj.ground, Background);

    obj.groundHeight = GroundRelHeight * frame::canvas_height();
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

    obj.projectileVelocity = (p2 - p1).Normalized() * ProjectileVelocityFactor;

    obj.velocityLength = 40.0f + 20.0f * (ProjectileVelocityFactor - 50.0f) / 150.0f;
}

void updateGravity(Objects& obj)
{
    obj.gravityLength = 40.0f + 20.0f * (GravityAcceleration - 5.0f) / 10.0f;
}
