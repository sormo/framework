#include "world.h"
#include "framework.h"
#include "point_type.h"

// scale from screen to world
constexpr float WorldScale = 50.0f;
constexpr float WorldScaleSqrt = 7.0710678f; // std::sqrtf(WorldScale);

const float World::RopeData::SegmentHeight = 10.0f;
const float World::RopeData::SegmentWidth = 15.0f;

b2Vec2 ConvertVector(const frame::vec2& v)
{
    return { v.x / WorldScale, -v.y / WorldScale };
}

b2Vec2 WorldScalePoint(const frame::vec2& p)
{
    return { p.x / WorldScale, p.y / WorldScale };
}

frame::vec2 WorldScalePoint(const b2Vec2& p)
{
    return { p.x * WorldScale, p.y * WorldScale };
}

World::World(const frame::vec2& gravity)
    : m_world({ gravity.x, gravity.y })
{

}

void World::SetGravity(const frame::vec2& gravity)
{
    m_world.SetGravity(b2Vec2(gravity.x, gravity.y));
}

World::Object World::CreateRectangle(const frame::vec2& position, float width, float height)
{
    b2PolygonShape rectangleShape{};
    rectangleShape.SetAsBox(width/(2.0f * WorldScale), height/(2.0f * WorldScale));

    ObjectData data;
    data.type = ObjectData::Type::Rectangle;
    data.shape.rectangle.width = width;
    data.shape.rectangle.height = height;

    return CreateObject(position, 0.0f, rectangleShape, std::move(data));
}

World::Object World::CreateRectangleEx(const frame::vec2& position, float angle, float width, float height)
{
    b2PolygonShape rectangleShape{};
    rectangleShape.SetAsBox(width / (2.0f * WorldScale), height / (2.0f * WorldScale));

    ObjectData data;
    data.type = ObjectData::Type::Rectangle;
    data.shape.rectangle.width = width;
    data.shape.rectangle.height = height;

    return CreateObject(position, angle, rectangleShape, std::move(data));
}

World::Object World::CreateCircle(const frame::vec2& position, float radius)
{
    b2CircleShape circleShape{};
    circleShape.m_radius = radius / WorldScale;
    circleShape.m_p.Set(0.0f, 0.0f);

    ObjectData data;
    data.type = ObjectData::Type::Circle;
    data.shape.circle.radius = radius;

    return CreateObject(position, 0.0f, circleShape, std::move(data));
}

World::Rope World::CreateRope(const std::vector<frame::vec2>& points, const color_type& color, Object* leftAttach, Object* rightAttach)
{
    // prepare objects

    Object lastObject = leftAttach ? *leftAttach : 0;

    std::vector<Object> objects;
    for (size_t i = 0; i < points.size() - 1; i++)
    {
        frame::vec2 vector = (points[i + 1] - points[i]).normalized();
        float angle = vector.angle();
        float distance = (points[i + 1] - points[i]).length();
        float centerDistance = RopeData::SegmentHeight * 0.5f;
        float anchorDistance = 0.0f;

        float ropeY = RopeData::SegmentHeight;
        if (ropeY > distance)
            ropeY = distance;
        else
            ropeY = distance / (std::roundf(distance / ropeY));

        auto createObject = [&]()
        {
            frame::vec2 center = points[i] + vector * centerDistance;
            frame::vec2 anchor = points[i] + vector * anchorDistance;
            // TODO why pi/2
            auto object = CreateRectangleEx(center, b2_pi / 2.0f - angle, RopeData::SegmentWidth, ropeY);
            SetStatic(object, false);
            SetDensity(object, 0.4f);
            SetFill(object, color_type::BLANK);

            b2Filter filter;
            filter.categoryBits = 0x2;
            m_objects[object].body->GetFixtureList()->SetFilterData(filter);

            if (lastObject)
                CreateRevoluteJoint(lastObject, object, anchor);

            return object;
        };

        while (centerDistance < distance)
        {
            auto object = createObject();

            objects.push_back(object);
            centerDistance += ropeY;
            anchorDistance += ropeY;
            lastObject = object;
        }
    }

    if (rightAttach)
    {
        CreateRevoluteJoint(objects.back(), *rightAttach, points.back());
    }

    auto setFilterMask = [this](Object* obj)
    {
        if (obj)
        {
            b2Filter filter;
            filter.maskBits = 0xfffd;
            m_objects[*obj].body->GetFixtureList()->SetFilterData(filter);
        }
    };
    setFilterMask(leftAttach);
    setFilterMask(rightAttach);

    // prepare distance joints

    // TODO constant increase
    for (int32_t i = 0; i < (int32_t)objects.size(); i += 3)
    {
        frame::vec2 anchorDown = GetPosition(objects[i]) + frame::vec2(0.0f, RopeData::SegmentHeight / 2.0f);
        for (int32_t j = i + 1; j < (int32_t)objects.size(); j++)
        {
            frame::vec2 anchor = GetPosition(objects[j]) - frame::vec2(0.0f, RopeData::SegmentHeight / 2.0f);
            float length = (float)(std::abs(i - j) + 1) * RopeData::SegmentHeight;

            CreateDistanceJointEx(objects[i], objects[j], anchorDown, anchor, length, 0.0f, length);
        }

        frame::vec2 anchorUp = GetPosition(objects[i]) - frame::vec2(0.0f, RopeData::SegmentHeight / 2.0f);
        for (int32_t j = i - 1; j >= 0; j--)
        {
            frame::vec2 anchor = GetPosition(objects[j]) + frame::vec2(0.0f, RopeData::SegmentHeight / 2.0f);
            float length = (float)(std::abs(i - j) + 1) * RopeData::SegmentHeight;

            CreateDistanceJointEx(objects[i], objects[j], anchorUp, anchor, length, 0.0f, length);
        }
    }

    RopeData data;
    data.segments = std::move(objects);
    data.fillColor = color;

    m_ropes[m_ropeCounter++] = std::move(data);
    m_layers[LayerDefault].ropes.push_back(m_ropeCounter - 1);

    return m_ropeCounter - 1;
}

World::Object World::CreateObject(const frame::vec2& position, float angle, b2Shape& shape, ObjectData&& data)
{
    b2BodyDef bodyDef{};
    bodyDef.position = WorldScalePoint(position);
    bodyDef.angle = angle;

    b2Body* body = m_world.CreateBody(&bodyDef);

    b2FixtureDef fixtureDef{};
    fixtureDef.friction = 0.8f;
    fixtureDef.density = 1.0f; // TODO
    fixtureDef.restitution = 0.1f;
    fixtureDef.shape = &shape;

    body->CreateFixture(&fixtureDef);

    data.body = body;
    data.fillColor = color_type::WHITE;

    m_objects[m_objectCounter++] = std::move(data);

    Object handle = m_objectCounter - 1;
    m_layers[LayerDefault].objects.push_back(handle);

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

void World::SetFill(Object obj, const color_type& color)
{
    m_objects[obj].fillColor = color;
}

void World::SetForce(Object obj, const frame::vec2& force)
{
    // TODO what is the scale factor for force, this is for sure not correct
    m_objects[obj].body->ApplyForceToCenter(b2Vec2(force.x, force.y), true);
}

void World::SetVelocity(Object obj, const frame::vec2& velocity)
{
    m_objects[obj].body->SetLinearVelocity(b2Vec2(velocity.x / WorldScaleSqrt, velocity.y / WorldScaleSqrt));
}

void World::SetLayer(Object obj, Layer layer)
{
    RemoveFromLayers(obj);

    m_layers[layer].objects.push_back(obj);
}

void World::SetBullet(Object obj, bool bullet)
{
    m_objects[obj].body->SetBullet(bullet);
}

frame::vec2 World::GetPosition(Object obj)
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

const color_type& World::GetFill(Object obj)
{
    return m_objects[obj].fillColor;
}

std::pair<frame::vec2, frame::vec2> World::GetPositions(Joint joint)
{
    return { WorldScalePoint(m_joints[joint]->GetAnchorA()), WorldScalePoint(m_joints[joint]->GetAnchorB()) };
}

void World::SetDensity(Object obj, float density)
{
    m_objects[obj].body->GetFixtureList()->SetDensity(density);
    m_objects[obj].body->ResetMassData();
}

void World::SetCollisionMask(Object obj, uint16_t mask)
{
    b2Filter data = m_objects[obj].body->GetFixtureList()->GetFilterData();
    data.maskBits = mask;

    m_objects[obj].body->GetFixtureList()->SetFilterData(data);
}

void World::Update()
{
    UpdateMouseJoints();

    m_world.Step(1.0f / 60.0f, 32, 16);
}

void World::Draw(Layer layer)
{
    assert(m_layers.find(layer) != std::end(m_layers));

    auto& layerObjects = m_layers[layer];

    for (const auto& obj : layerObjects.objects)
    {
        assert(m_objects.find(obj) != std::end(m_objects));

        DrawObject(m_objects[obj]);
    }

    for (const auto& rope : layerObjects.ropes)
    {
        assert(m_ropes.find(rope) != std::end(m_ropes));

        DrawRope(m_ropes[rope]);
    }

    // debug
    //DrawJointsDebug();
}

void World::DrawObject(const ObjectData& data)
{
    auto position = WorldScalePoint(data.body->GetPosition());

    if (data.type == ObjectData::Type::Rectangle)
    {
        frame::draw_rectangle_ex(position,
            data.body->GetAngle(),
            data.shape.rectangle.width,
            data.shape.rectangle.height,
            data.fillColor, 0.0f, color_type::BLANK);
    }
    else
    {
        frame::draw_circle_ex(position,
            data.body->GetAngle(),
            data.shape.circle.radius,
            data.fillColor, 0.0f, color_type::BLANK);
    }
}

void World::DrawRope(const RopeData& data)
{
    for (const auto& rope : m_ropes)
    {
        std::vector<frame::vec2> points(data.segments.size() + 2);

        points[0] = WorldScalePoint(m_objects[data.segments[0]].body->GetWorldPoint(WorldScalePoint(frame::vec2(0.0f, -RopeData::SegmentHeight))));

        for (size_t i = 0; i < data.segments.size(); i++)
            points[i + 1] = WorldScalePoint(m_objects[data.segments[i]].body->GetPosition());

        points.back() = WorldScalePoint(m_objects[data.segments.back()].body->GetWorldPoint(WorldScalePoint(frame::vec2(0.0f, RopeData::SegmentHeight))));;

        frame::draw_bezier_polyline_ex(points, RopeData::SegmentHeight * 1.0f, data.fillColor);
    }
}

void World::DrawJointsDebug()
{
    for (const auto& joint : m_joints)
    {
        if (joint.second->GetType() != e_revoluteJoint)
            continue;

        auto p1 = WorldScalePoint(joint.second->GetAnchorA());
        auto p2 = WorldScalePoint(joint.second->GetAnchorB());

        frame::draw_circle_ex(p1, 0.0f, 5.0f, color_type::RGBf(1.0f, 0.0f, 0.0f, 0.5f), 0.0f, color_type::BLANK);
        frame::draw_circle_ex(p2, 0.0f, 5.0f, color_type::RGBf(1.0f, 0.0f, 0.0f, 0.5f), 0.0f, color_type::BLANK);
        frame::draw_line_solid(p1, p2, color_type::RGBf(1.0f, 1.0f, 1.0f, 0.5f));
    }
}

void World::SetLayerRope(Rope rope, Layer layer)
{
    RemoveFromLayersRope(rope);

    m_layers[layer].ropes.push_back(rope);
}

void World::RemoveFromLayersRope(Rope rope)
{
    for (auto& [layer, layerObjects] : m_layers)
    {
        auto it = std::find(std::begin(layerObjects.ropes), std::end(layerObjects.ropes), rope);
        if (it != std::end(layerObjects.ropes))
        {
            layerObjects.ropes.erase(it);
            break;
        }
    }
}

void World::RemoveFromLayers(Object obj)
{
    for (auto& [layer, layerObjects] : m_layers)
    {
        auto it = std::find(std::begin(layerObjects.objects), std::end(layerObjects.objects), obj);
        if (it != std::end(layerObjects.objects))
        {
            layerObjects.objects.erase(it);
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

World::Object World::GetObjectFromBody(b2Body* body)
{
    for (auto& [obj, data] : m_objects)
    {
        if (data.body == body)
            return obj;
    }
    assert(false);
    return -1;
}

class QueryObjectsCallback : public b2QueryCallback
{
    virtual bool ReportFixture(b2Fixture* f)
    {
        if (f->TestPoint(point))
            bodies.push_back(f->GetBody());
        return true;
    }
public:
    std::vector<b2Body*> bodies;
    b2Vec2 point;
};

std::vector<World::Object> World::QueryObjects(const frame::vec2& position)
{
    QueryObjectsCallback callback;
    callback.point = WorldScalePoint(position);

    b2AABB aabb;
    aabb.lowerBound = callback.point - b2Vec2(0.1f, 0.1f);
    aabb.upperBound = callback.point + b2Vec2(0.1f, 0.1f);

    m_world.QueryAABB(&callback, aabb);

    std::vector<World::Object> result;
    for (auto body : callback.bodies)
        result.push_back(GetObjectFromBody(body));

    return result;
}

void World::EnsureGroundObjectCreated()
{
    if (m_ground)
        return;

    b2BodyDef def{};
    m_ground = m_world.CreateBody(&def);
}

World::Joint World::CreateMouseJoint(Object obj, const frame::vec2& target)
{
    EnsureGroundObjectCreated();

    static const float FrequencyHz = 5.0f;
    static const float DampingRatio = 0.7f;
    static const float MaxForceFactor = 1000.0f;

    b2MouseJointDef def;
    def.bodyA = m_ground;
    def.bodyB = m_objects[obj].body;
    def.target = WorldScalePoint(target);
    def.maxForce = MaxForceFactor * m_objects[obj].body->GetMass();
    b2LinearStiffness(def.stiffness, def.damping, FrequencyHz, DampingRatio, def.bodyA, def.bodyB);

    m_joints[m_jointCounter++] = m_world.CreateJoint(&def);
    def.bodyB->SetAwake(true);

    return m_jointCounter - 1;
}

World::Joint World::CreateRevoluteJoint(Object obj1, Object obj2, const frame::vec2& target)
{
    b2RevoluteJointDef def;
    def.bodyA = m_objects[obj1].body;
    def.bodyB = m_objects[obj2].body;
    def.collideConnected = false;
    def.localAnchorA = m_objects[obj1].body->GetLocalPoint(WorldScalePoint(target));
    def.localAnchorB = m_objects[obj2].body->GetLocalPoint(WorldScalePoint(target));

    m_joints[m_jointCounter++] = m_world.CreateJoint(&def);

    return m_jointCounter - 1;
}

World::Joint World::CreateDistanceJoint(Object obj1, Object obj2, const frame::vec2& point1, const frame::vec2& point2, bool allowSmallerDistance)
{
    b2DistanceJointDef def;
    def.bodyA = m_objects[obj1].body;
    def.bodyB = m_objects[obj2].body;
    def.collideConnected = false;
    def.localAnchorA = m_objects[obj1].body->GetLocalPoint(WorldScalePoint(point1));
    def.localAnchorB = m_objects[obj2].body->GetLocalPoint(WorldScalePoint(point2));
    def.length = b2Distance(WorldScalePoint(point1), WorldScalePoint(point2));
    def.maxLength = def.length;
    def.minLength = allowSmallerDistance ? 0.0f : def.length;
    //b2LinearStiffness(def.stiffness, def.damping, 1.0f, 1.0f, def.bodyA, def.bodyB);

    m_joints[m_jointCounter++] = m_world.CreateJoint(&def);

    return m_jointCounter - 1;
}

World::Joint World::CreateDistanceJointEx(Object obj1, Object obj2, const frame::vec2& point1, const frame::vec2& point2, float length, float minLength, float maxLength)
{
    b2DistanceJointDef def;
    def.bodyA = m_objects[obj1].body;
    def.bodyB = m_objects[obj2].body;
    def.collideConnected = false;
    def.localAnchorA = m_objects[obj1].body->GetLocalPoint(WorldScalePoint(point1));
    def.localAnchorB = m_objects[obj2].body->GetLocalPoint(WorldScalePoint(point2));
    def.length = length;
    def.maxLength = maxLength;
    def.minLength = minLength;
    //b2LinearStiffness(def.stiffness, def.damping, 1.0f, 1.0f, def.bodyA, def.bodyB);

    m_joints[m_jointCounter++] = m_world.CreateJoint(&def);

    return m_jointCounter - 1;
}

void World::DestroyJoint(Joint joint)
{
    m_world.DestroyJoint(m_joints[joint]);
    m_joints.erase(joint);
}

void World::UpdateMouseJoints()
{
    for (auto& [_, joint] : m_joints)
    {
        if (joint->GetType() == e_mouseJoint)
        {
            //b2MouseJoint* mouseJoint = dynamic_cast<b2MouseJoint*>(joint);
            b2MouseJoint* mouseJoint = (b2MouseJoint*)joint;
            mouseJoint->SetTarget(WorldScalePoint(frame::get_mouse_world_position()));
        }
    }
}

DebugDraw::DebugDraw()
{
    SetFlags(e_shapeBit);
}

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    std::vector<frame::vec2> points(vertexCount);
    for (int32_t i = 0; i < vertexCount; i++)
        points[i] = WorldScalePoint(vertices[i]);
    frame::draw_polygon({}, points.data(), points.size(), color_type::RGBf(color.r, color.g, color.b, color.a));
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    std::vector<frame::vec2> points(vertexCount);
    for (int32_t i = 0; i < vertexCount; i++)
        points[i] = WorldScalePoint(vertices[i]);
    frame::draw_polygon({}, points.data(), points.size(), color_type::RGBf(color.r, color.g, color.b, color.a));
}

void DebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
    frame::draw_circle_ex(WorldScalePoint(center), 0.0f, radius * WorldScale, color_type::RGBf(color.r, color.g, color.b, color.a), 0.0f, color_type::BLANK);
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
    frame::draw_circle_ex(WorldScalePoint(center), 0.0f, radius * WorldScale, color_type::RGBf(color.r, color.g, color.b, color.a), 0.0f, color_type::BLANK);
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    frame::draw_line_solid(WorldScalePoint(p1), WorldScalePoint(p2), color_type::RGBf(color.r, color.g, color.b, color.a));
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{

}

void DebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
    frame::draw_circle_ex(WorldScalePoint(p), 0.0f, 2.0f, color_type::RGBf(color.r, color.g, color.b, color.a), 0.0f, color_type::BLANK);
}