#include "framework.h"
#include "imgui.h"
#include "world.h"

#include <box2d/b2_rope.h>

int32_t circles_count = 0;
int32_t squares_count = 0;

World world;
World::Joint mouse_joint = 0;

struct VerletNode
{
    b2Vec2 position;
    b2Vec2 oldPosition;

    VerletNode() {}
    VerletNode(const b2Vec2& startPos)
        : position(startPos), oldPosition(startPos)
    {
    }
};

struct Separation
{
    b2Body* body;
    b2Vec2 vector;
};

std::vector<Separation> g_separations;

struct Rope
{
    static const int iterations = 80;
    static const int totalNodes = 160;
    const float nodeDistance = .1f;
    const b2Vec2 gravity = b2Vec2(0.0f, -9.89f);

    b2Vec2 position = b2Vec2(0.0f, 0.0f);

    VerletNode nodes[totalNodes];

    b2Body* bodies[totalNodes] = {};

    Rope(const b2Vec2& position)
        : position(position)
    {
        b2Vec2 tmpPosition = position;
        for (int i = 0; i < totalNodes; i++)
        {
            nodes[i] = VerletNode(tmpPosition);
            tmpPosition.y -= nodeDistance;
        }

        CreateFixtures();
    }

    void CreateFixtures()
    {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;

        b2CircleShape shape;
        shape.m_radius = nodeDistance * 0.5f;
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 0.5f;
        fixtureDef.userData.type = b2FixtureUserData::Type::Rope;

        for (int i = 0; i < totalNodes; i++)
        {
            bodyDef.position = nodes[i].position;
            bodies[i] = world.m_world.CreateBody(&bodyDef);

            fixtureDef.userData.ropeIndex = i;
            auto fixture = bodies[i]->CreateFixture(&fixtureDef);
        }
    }

    void Draw()
    {
        for (int i = 0; i < totalNodes; i++)
        {
            //auto position = WorldScalePoint(nodes[i].position);
            auto position = WorldScalePoint(bodies[i]->GetPosition());
            frame::draw_circle(position, 2.0f, Color::ORANGE);
        }
    }

    void Update(float dt)
    {
        Simulate(dt);
        for (int i = 0; i < iterations; i++)
        {
            ApplyConstraints();

            const_cast<b2ContactManager&>(world.m_world.GetContactManager()).FindNewContacts();

            for (auto& separation : g_separations)
                separation.body->SetTransform(separation.body->GetPosition() + separation.vector, separation.body->GetAngle());
            g_separations.clear();
        }
    }

    void Simulate(float dt)
    {
        b2Vec2 gravityDt = gravity;
        gravityDt *= (dt * dt);

        for (int i = 0; i < totalNodes; i++)
        {
            VerletNode& node = nodes[i];

            b2Vec2 temp = node.position;
            node.position += (node.position - node.oldPosition) + gravityDt;
            node.oldPosition = temp;
        }
    }

    void ApplyConstraints()
    {
        for (int i = 0; i < totalNodes - 1; i++)
        {
            VerletNode& node1 = nodes[i];
            VerletNode& node2 = nodes[i + 1];

            // First node follows the mouse, for debugging.
            if (i == 0 && frame::mouse_pressed(frame::mouse_button::middle))
            {
                node1.position = WorldScalePoint(frame::mouse_canvas_position());
            }

            // Current distance between rope nodes.
            b2Vec2 diff;
            diff.x = node1.position.x - node2.position.x;
            diff.y = node1.position.y - node2.position.y;
            float dist = b2Distance(node1.position, node2.position);
            float difference = 0;
            // Guard against divide by 0.
            if (dist > 0)
            {
                difference = (nodeDistance - dist) / dist;
            }

            diff *= (0.5f * difference);

            node1.position += diff;
            node2.position -= diff;

            bodies[i]->SetTransform(node1.position, 0.0f);
            bodies[i + 1]->SetTransform(node2.position, 0.0f);
        }
    }
};

std::unique_ptr<Rope> g_rope2;

void create_square()
{
    auto rect = world.CreateRectangle(frame::mouse_screen_position(), 10.0f, 10.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, Color::RGB(227, 142, 31));

    squares_count++;
}

void create_circle()
{
    auto rect = world.CreateCircle(frame::mouse_screen_position(), 5.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, Color::RGB(177, 242, 235));

    circles_count++;
}

void create_ground()
{
    auto ground = world.CreateRectangle(frame::rel_pos(0.5f, 0.0f), frame::screen_width(), 20.0f);
    world.SetStatic(ground, true);
    world.SetFill(ground, Color::RGB(80, 80, 80));
}

class ContactListener : public b2ContactListener
{
public:
    //void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override
    void BeginContact(b2Contact* contact) override
    {
        if (!contact->IsTouching())
            return;

        if (contact->GetFixtureA()->GetUserData().type == b2FixtureUserData::Type::Rope &&
            contact->GetFixtureB()->GetUserData().type == b2FixtureUserData::Type::Rope)
            return;

        if (contact->GetFixtureA()->GetUserData().type != b2FixtureUserData::Type::Rope &&
            contact->GetFixtureB()->GetUserData().type != b2FixtureUserData::Type::Rope)
            return;

        b2Fixture* ropeFixture = contact->GetFixtureA()->GetUserData().type == b2FixtureUserData::Type::Rope ? contact->GetFixtureA() : contact->GetFixtureB();
        b2Fixture* otherFixture = contact->GetFixtureA()->GetUserData().type != b2FixtureUserData::Type::Rope ? contact->GetFixtureA() : contact->GetFixtureB();

        b2WorldManifold manifold;
        contact->GetWorldManifold(&manifold);

        b2Vec2 separation = manifold.normal;
        separation *= manifold.separations[0];

        if (contact->GetFixtureA() == otherFixture)
        {
            g_separations.push_back({ ropeFixture->GetBody(), -separation });
            //ropeFixture->GetBody()->SetTransform(ropeFixture->GetBody()->GetPosition() + separation, ropeFixture->GetBody()->GetAngle());
        }
        else
        {
            g_separations.push_back({ ropeFixture->GetBody(), separation });
            //ropeFixture->GetBody()->SetTransform(ropeFixture->GetBody()->GetPosition() - separation, ropeFixture->GetBody()->GetAngle());
        }
        contact->SetEnabled(false);
    }

} g_listener;

void setup()
{
    g_rope2 = std::make_unique<Rope>(WorldScalePoint(Point(400.0f, 300.0f)));

    world.m_world.SetContactListener(&g_listener);

    create_ground();

    frame::mouse_press_register(frame::mouse_button::left, []()
    {
        if (rand() % 2)
            create_square();
        else
            create_circle();
    });

    frame::mouse_press_register(frame::mouse_button::right, []()
    {
        auto objects = world.QueryObjects(frame::mouse_canvas_position());
        if (!objects.empty())
            mouse_joint = world.CreateMouseJoint(objects[0], frame::mouse_canvas_position());
    });

    frame::mouse_release_register(frame::mouse_button::right, []()
    {
        if (mouse_joint)
            world.DestroyJoint(mouse_joint);
        mouse_joint = 0;
    });

    frame::canvas_background(Color::RGBf(0.1f, 0.1f, 0.1f));
}

void draw_gui()
{
    bool open = true;
    ImGui::SetNextWindowPos({ 10.0f, 10.0f });
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Circles");
    ImGui::SameLine();
    ImGui::Text("%d", circles_count);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Squares");
    ImGui::SameLine();
    ImGui::Text("%d", squares_count);
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Use right mouse button to grab objects");

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void update()
{
    draw_gui();
    
    world.Update();

    g_rope2->Update(1.0f/ 60.0f);

    world.Draw();

    g_rope2->Draw();
}
