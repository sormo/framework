#pragma once
#include "point.h"
#include "color.h"
#include <box2d/box2d.h>
#include <nanovg.h>
#include <unordered_map>
#include <vector>

class DebugDraw : public b2Draw
{
public:
    DebugDraw();
    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
    void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
    void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
    void DrawTransform(const b2Transform& xf) override;
    void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;
};

class World
{
public:
    using Object = int32_t;
    using Layer = int32_t;
    using Joint = int32_t;

    static constexpr Layer LayerDefault = 0;

    World(const Point& gravity = { 0.0f, -9.89f });

    void SetGravity(const Point& gravity);

    Object CreateRectangle(const Point& position, float width, float height);
    Object CreateRectangleEx(const Point& position, float angle, float width, float height);
    Object CreateCircle(const Point& position, float radius);

    void Destroy(Object object);

    Point GetPosition(Object obj);
    float GetRotation(Object obj);
    float GetMass(Object obj);
    const Color& GetFill(Object obj);

    std::pair<Point, Point> GetPositions(Joint joint);

    void SetStatic(Object obj, bool isStatic);
    void SetFill(Object obj, const Color& color);
    void SetForce(Object obj, const Point& force);
    void SetVelocity(Object obj, const Point& velocity);
    void SetLayer(Object obj, Layer layer = LayerDefault);
    void SetBullet(Object obj, bool bullet);
    void SetDensity(Object obj, float density);

    void SetCollisionMask(Object obj, uint16_t mask);

    void Update();
    void Draw(Layer layer = LayerDefault);

    void Clear();

    std::vector<Object> QueryObjects(const Point& position);

    // target is initial position on object which will be dragged to mouse
    Joint CreateMouseJoint(Object obj, const Point& target);
    void DestroyJoint(Joint joint);
    // target is position of joint
    Joint CreateRevoluteJoint(Object obj1, Object obj2, const Point& target);
    Joint CreateDistanceJoint(Object obj1, Object obj2, const Point& point1, const Point& point2, bool allowSmallerDistance);
    Joint CreateDistanceJointEx(Object obj1, Object obj2, const Point& point1, const Point& point2, float length, float minLength, float maxLength);

private:
    b2World m_world;
    Object m_objectCounter = 1;
    Joint m_jointCounter = 1;

    struct ObjectData
    {
        b2Body* body;
        Color fillColor;

        enum class Type
        {
            Rectangle,
            Circle
        } type;

        union
        {
            struct
            {
                float width;
                float height;
            } rectangle;
            struct
            {
                float radius;
            } circle;
        } shape;    
    };

    Object CreateObject(const Point& position, float angle, b2Shape& shape, ObjectData&& data);
    void DrawObject(const ObjectData& data);
    void DrawJointsDebug();

    void RemoveFromLayers(Object obj);

    Object GetObjectFromBody(b2Body* body);

    // TODO joints are attached to bodies, if body is destroyed, joints may get destroyed also
    // need to cleanup on body destroy

    std::unordered_map<Object, ObjectData> m_objects;
    std::unordered_map<Joint, b2Joint*> m_joints;
    std::unordered_map<Layer, std::vector<Object>> m_layers;

    b2Body* m_ground = nullptr;
    void EnsureGroundObjectCreated();

    void UpdateMouseJoints();
};
