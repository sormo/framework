#pragma once
#include "point_type.h"
#include "color_type.h"
#include <box2d/box2d.h>
#include <nanovg.h>
#include <unordered_map>
#include <vector>

b2Vec2 ConvertVector(const point_type<float>& v);
b2Vec2 WorldScalePoint(const point_type<float>& p);
point_type<float> WorldScalePoint(const b2Vec2& p);

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
    using Rope = int32_t;

    static constexpr Layer LayerDefault = 0;

    World(const point_type<float>& gravity = { 0.0f, -9.89f });

    void SetGravity(const point_type<float>& gravity);

    Object CreateRectangle(const point_type<float>& position, float width, float height);
    Object CreateRectangleEx(const point_type<float>& position, float angle, float width, float height);
    Object CreateCircle(const point_type<float>& position, float radius);

    void Destroy(Object object);

    point_type<float> GetPosition(Object obj);
    float GetRotation(Object obj);
    float GetMass(Object obj);
    const color_type& GetFill(Object obj);

    std::pair<point_type<float>, point_type<float>> GetPositions(Joint joint);

    void SetStatic(Object obj, bool isStatic);
    void SetFill(Object obj, const color_type& color);
    void SetForce(Object obj, const point_type<float>& force);
    void SetVelocity(Object obj, const point_type<float>& velocity);
    void SetLayer(Object obj, Layer layer = LayerDefault);
    void SetLayerRope(Rope rope, Layer layer = LayerDefault);
    void SetBullet(Object obj, bool bullet);
    void SetDensity(Object obj, float density);

    void SetCollisionMask(Object obj, uint16_t mask);

    void Update();
    void Draw(Layer layer = LayerDefault);

    void Clear();

    std::vector<Object> QueryObjects(const point_type<float>& position);

    // target is initial position on object which will be dragged to mouse
    Joint CreateMouseJoint(Object obj, const point_type<float>& target);
    void DestroyJoint(Joint joint);
    // target is position of joint
    Joint CreateRevoluteJoint(Object obj1, Object obj2, const point_type<float>& target);
    Joint CreateDistanceJoint(Object obj1, Object obj2, const point_type<float>& point1, const point_type<float>& point2, bool allowSmallerDistance);
    Joint CreateDistanceJointEx(Object obj1, Object obj2, const point_type<float>& point1, const point_type<float>& point2, float length, float minLength, float maxLength);

    Rope CreateRope(const std::vector<point_type<float>>& points, const color_type& color, Object* leftAttach, Object* rightAttach);

    //private:
    b2World m_world;
    Object m_objectCounter = 1;
    Joint m_jointCounter = 1;
    Rope m_ropeCounter = 1;

    struct ObjectData
    {
        b2Body* body;
        color_type fillColor;

        enum class Type
        {
            Rectangle,
            Circle,
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

    struct RopeData
    {
        static const float SegmentHeight;
        static const float SegmentWidth;

        color_type fillColor;
        std::vector<Object> segments;
    };

    Object CreateObject(const point_type<float>& position, float angle, b2Shape& shape, ObjectData&& data);
    void DrawObject(const ObjectData& data);
    void DrawRope(const RopeData& data);
    void DrawJointsDebug();

    void RemoveFromLayers(Object obj);
    void RemoveFromLayersRope(Rope obj);

    Object GetObjectFromBody(b2Body* body);

    // TODO joints are attached to bodies, if body is destroyed, joints may get destroyed also
    // need to cleanup on body destroy

    // TODO separate layers and whole drawing from world ???
    struct LayerData
    {
        std::vector<Object> objects;
        std::vector<Rope> ropes;
    };

    std::unordered_map<Object, ObjectData> m_objects;
    std::unordered_map<Joint, b2Joint*> m_joints;
    std::unordered_map<Rope, RopeData> m_ropes;
    std::unordered_map<Layer, LayerData> m_layers;

    b2Body* m_ground = nullptr;
    void EnsureGroundObjectCreated();

    void UpdateMouseJoints();
};
