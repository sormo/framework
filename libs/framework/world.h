#pragma once
#include "point.h"
#include <box2d/box2d.h>
#include <nanovg.h>
#include <unordered_map>
#include <vector>

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
    Object CreateCircle(const Point& position, float radius);

    void Destroy(Object object);

    Point GetPosition(Object obj);
    float GetRotation(Object obj);
    float GetMass(Object obj);
    const NVGcolor& GetFill(Object obj);

    void SetStatic(Object obj, bool isStatic);
    void SetFill(Object obj, const NVGcolor& color);
    void SetForce(Object obj, const Point& force);
    void SetVelocity(Object obj, const Point& velocity);
    void SetLayer(Object obj, Layer layer = LayerDefault);
    void SetBullet(Object obj, bool bullet); 

    void Update();
    void Draw(Layer layer = LayerDefault);

    void Clear();

    std::vector<Object> QueryObjects(const Point& position);

    // target is initial position which will be dragged to mouse
    Joint CreateMouseJoint(Object obj, const Point& target);
    void DestroyJoint(Joint joint);

private:
    b2World m_world;
    Object m_objectCounter = 1;
    Joint m_jointCounter = 1;

    struct ObjectData
    {
        b2Body* body;
        NVGcolor fillColor;

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

    Object CreateObject(const Point& position, b2Shape& shape, ObjectData&& data);
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
