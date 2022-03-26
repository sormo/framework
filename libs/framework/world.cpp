#include "world.h"
#include "framework.h"
#include "point.h"

// scale from screen to world
constexpr float WorldScale = 50.0f;
constexpr float WorldScaleSqrt = 7.0710678f; // std::sqrtf(WorldScale);

b2Vec2 ConvertVector(const Point& v)
{
    return { v.x() / WorldScale, -v.y() / WorldScale };
}

b2Vec2 WorldScalePoint(const Point& p)
{
    return { p.x() / WorldScale, p.y() / WorldScale };
}

Point WorldScalePoint(const b2Vec2& p)
{
    return { p.x * WorldScale, p.y * WorldScale };
}

World::World(const Point& gravity)
    : m_world({ gravity.x(), gravity.y() })
{

}

void World::SetGravity(const Point& gravity)
{
    m_world.SetGravity(b2Vec2(gravity.x(), gravity.y()));
}

World::Object World::CreateRectangle(const Point& position, float width, float height)
{
    b2PolygonShape rectangleShape{};
    rectangleShape.SetAsBox(width/(2.0f * WorldScale), height/(2.0f * WorldScale));

    ObjectData data;
    data.type = ObjectData::Type::Rectangle;
    data.shape.rectangle.width = width;
    data.shape.rectangle.height = height;

    return CreateObject(position, rectangleShape, std::move(data));
}

World::Object World::CreateCircle(const Point& position, float radius)
{
    b2CircleShape circleShape{};
    circleShape.m_radius = radius / WorldScale;
    circleShape.m_p.Set(0.0f, 0.0f);

    ObjectData data;
    data.type = ObjectData::Type::Circle;
    data.shape.circle.radius = radius;

    return CreateObject(position, circleShape, std::move(data));
}

World::Object World::CreateObject(const Point& position, b2Shape& shape, ObjectData&& data)
{
    b2BodyDef bodyDef{};
    bodyDef.position = WorldScalePoint(position);
    bodyDef.type = b2_staticBody;

    b2Body* body = m_world.CreateBody(&bodyDef);

    b2FixtureDef fixtureDef{};
    fixtureDef.isSensor = false;
    fixtureDef.friction = 0.8f;
    fixtureDef.density = 12.0f;
    fixtureDef.restitution = 0.1f;
    fixtureDef.shape = &shape;

    body->CreateFixture(&fixtureDef);

    data.body = body;
    data.fillColor = nvgRGB(255, 255, 255);

    m_objects[m_objectCounter++] = std::move(data);

    Object handle = m_objectCounter - 1;
    m_layers[LayerDefault].push_back(handle);

    return handle;
}

void World::Destroy(Object object)
{
    m_world.DestroyBody(m_objects[object].body);
    m_objects.erase(object);
    RemoveFromLayers(object);
}

void World::SetStatic(Object obj, bool isStatic)
{
    m_objects[obj].body->SetType(isStatic ? b2_staticBody : b2_dynamicBody);
}

void World::SetFill(Object obj, const NVGcolor& color)
{
    m_objects[obj].fillColor = color;
}

void World::SetForce(Object obj, const Point& force)
{
    // TODO what is the scale factor for force, this is for sure not correct
    m_objects[obj].body->ApplyForceToCenter(b2Vec2(force.x(), force.y()), true);
}

void World::SetVelocity(Object obj, const Point& velocity)
{
    m_objects[obj].body->SetLinearVelocity(b2Vec2(velocity.x() / WorldScaleSqrt, velocity.y() / WorldScaleSqrt));
}

void World::SetLayer(Object obj, Layer layer)
{
    RemoveFromLayers(obj);

    m_layers[layer].push_back(obj);
}

void World::SetBullet(Object obj, bool bullet)
{
    m_objects[obj].body->SetBullet(bullet);
}

Point World::GetPosition(Object obj)
{
    return WorldScalePoint(m_objects[obj].body->GetPosition());
}

float World::GetRotation(Object obj)
{
    // TODO verify if angle should be negative
    return m_objects[obj].body->GetAngle();
}

float World::GetMass(Object obj)
{
    return m_objects[obj].body->GetMass();
}

const NVGcolor& World::GetFill(Object obj)
{
    return m_objects[obj].fillColor;
}

void World::Update()
{
    m_world.Step(1.0f / 60.0f, 8, 3);
}

void World::Draw(Layer layer)
{
    assert(m_layers.find(layer) != std::end(m_layers));

    std::vector<Object>& objects = m_layers[layer];

    for (const auto& obj : objects)
    {
        assert(m_objects.find(obj) != std::end(m_objects));

        DrawObject(m_objects[obj]);
    }
}

void World::DrawObject(const ObjectData& data)
{
    auto position = WorldScalePoint(data.body->GetPosition());

    if (data.type == ObjectData::Type::Rectangle)
    {
        frame::rectangle(position,
            data.body->GetAngle(),
            data.shape.rectangle.width,
            data.shape.rectangle.height,
            data.fillColor);
    }
    else
    {
        frame::circle(position,
            data.body->GetAngle(),
            data.shape.circle.radius,
            data.fillColor);
    }
}

void World::RemoveFromLayers(Object obj)
{
    for (auto& [layer, objects] : m_layers)
    {
        auto it = std::find(std::begin(objects), std::end(objects), obj);
        if (it != std::end(objects))
        {
            objects.erase(it);
            break;
        }
    }
}

void World::Clear()
{
    for (auto& [_, obj] : m_objects)
        m_world.DestroyBody(obj.body);

    m_objects.clear();
    m_layers.clear();
}
