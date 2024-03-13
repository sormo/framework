#include "framework.h"
#include "imgui.h"
#include "world.h"

int32_t circles_count = 0;
int32_t squares_count = 0;

World world;
World::Joint mouse_joint = 0;

struct
{
    frame::vec2 circle2_hold_position;
    frame::vec2 circle2_hold_size;

    float radius1;
    float radius2;

    World::Object top;
    World::Object circle1;
    World::Object circle2;
    World::Object weight1;
    World::Object weight2;
    World::Rope rope;
    std::vector<World::Joint> joints;

} kladkostroj;

void create_square()
{
    auto rect = world.CreateRectangle(frame::get_mouse_world_position(), 10.0f, 10.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, frame::col4::RGB(227, 142, 31));

    squares_count++;
}

void create_circle()
{
    auto rect = world.CreateCircle(frame::get_mouse_world_position(), 5.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, frame::col4::RGB(177, 242, 235));

    circles_count++;
}

void create_ground()
{
    auto ground = world.CreateRectangle(frame::get_world_position_screen_relative({ 0.5f, 0.0f }), frame::get_screen_size().x, 20.0f);
    world.SetStatic(ground, true);
    world.SetFill(ground, frame::col4::RGB(80, 80, 80));
}

void draw_kladkostroj()
{
    world.Draw(-10);
    world.Draw(0);

    frame::draw_rectangle_ex(world.GetPosition(kladkostroj.circle1), world.GetRotation(kladkostroj.circle1), kladkostroj.radius1 * 2.0f, 0.2f * kladkostroj.radius1, frame::col4::RGB(150, 150, 150), 0.0f, frame::col4::BLANK);
    frame::draw_rectangle_ex(world.GetPosition(kladkostroj.circle2), world.GetRotation(kladkostroj.circle2), kladkostroj.radius2 * 2.0f, 0.2f * kladkostroj.radius2, frame::col4::RGB(150, 150, 150), 0.0f, frame::col4::BLANK);

    world.Draw(10);

    frame::draw_rectangle(kladkostroj.circle2_hold_position, kladkostroj.circle2_hold_size.x, kladkostroj.circle2_hold_size.y, frame::col4::WHITE);

    for (auto joint : kladkostroj.joints)
        frame::draw_circle(world.GetPositions(joint).first, 2.0f, frame::col4::DARKGRAY);
}

void create_kladkostroj()
{
    auto world_size = frame::get_world_size();

    kladkostroj.radius1 = 0.05f * world_size.x;
    kladkostroj.radius2 = 0.05f * world_size.x;
    const float centerx = 0.5f * world_size.x;
    const float topy = 0.95f * world_size.y;
    const float weight_y_position = 0.4f * world_size.y;

    // create top

    const frame::vec2 top_position(0.5f * world_size.x, topy + (world_size.y - topy) / 2.0f);
    kladkostroj.top = world.CreateRectangle(top_position, world_size.x, world_size.y - topy);
    world.SetFill(kladkostroj.top, frame::col4::DARKGRAY);

    // create circle1

    const frame::vec2 position1(centerx - kladkostroj.radius1, weight_y_position + 2.5f * kladkostroj.radius1);
    kladkostroj.circle1 = world.CreateCircle(position1, kladkostroj.radius1);
    world.SetStatic(kladkostroj.circle1, false);
    world.SetFill(kladkostroj.circle1, frame::col4::LIGHTGRAY);

    // create circle2

    const frame::vec2 position2(centerx + kladkostroj.radius2, topy - 2.0f * kladkostroj.radius2);
    kladkostroj.circle2 = world.CreateCircle(position2, kladkostroj.radius2);
    world.SetStatic(kladkostroj.circle2, false);
    world.SetFill(kladkostroj.circle2, frame::col4::LIGHTGRAY);
    kladkostroj.joints.push_back(world.CreateRevoluteJoint(kladkostroj.circle2, kladkostroj.top, position2));

    // prepare hold of circle2 for drawing 

    kladkostroj.circle2_hold_position = frame::vec2(position2.x, position2.y + (topy - position2.y) / 2.0f);
    kladkostroj.circle2_hold_size = frame::vec2(kladkostroj.radius2 * 0.3f, (topy - position2.y) * 1.1f);

    // create weight1

    const float weight1_width = 1.75f * kladkostroj.radius2;
    const float weight1_height = 0.75f * kladkostroj.radius2;
    const frame::vec2 weight1_position = frame::vec2(centerx - kladkostroj.radius1, weight_y_position);
    kladkostroj.weight1 = world.CreateRectangle(weight1_position, weight1_width, weight1_height);
    world.SetStatic(kladkostroj.weight1, false);
    world.SetFill(kladkostroj.weight1, frame::col4::RGB(0, 76, 153));
    world.SetDensity(kladkostroj.weight1, 10.0f);

    // create weight2

    const float weight2_width = 1.75f * kladkostroj.radius2;
    const float weight2_height = 0.75f * kladkostroj.radius2;
    const frame::vec2 weight2_position = frame::vec2(centerx + 2.0f * kladkostroj.radius2, weight_y_position);
    kladkostroj.weight2 = world.CreateRectangle(weight2_position, weight2_width, weight2_height);
    world.SetStatic(kladkostroj.weight2, false);
    world.SetFill(kladkostroj.weight2, frame::col4::RGB(0, 76, 153));
    world.SetDensity(kladkostroj.weight2, 6.06f);

    // create hold for weight1

    const frame::vec2 weight1_connection_point = weight1_position + frame::vec2(0.0f, weight1_height / 2.0f);
    const float weight1_connection_width = 0.2f * kladkostroj.radius1;
    const float weight1_connection_height = (weight1_connection_point - position1).length() * 1.1f;
    const frame::vec2 weight1_connection_position = position1 + (weight1_connection_point - position1) / 2.0f;
    auto weight1_connection = world.CreateRectangle(weight1_connection_position, weight1_connection_width, weight1_connection_height);
    world.SetStatic(weight1_connection, false);
    world.SetCollisionMask(weight1_connection, 0);
    world.SetDensity(weight1_connection, 0.01f);
    world.SetLayer(weight1_connection, 10);
    kladkostroj.joints.push_back(world.CreateRevoluteJoint(kladkostroj.weight1, weight1_connection, weight1_position + frame::vec2(0.0f, weight1_height / 2.0f)));
    kladkostroj.joints.push_back(world.CreateRevoluteJoint(kladkostroj.circle1, weight1_connection, position1));

    // create rope

    std::vector<frame::vec2> rope;
    rope.push_back({ centerx - 2.0f * kladkostroj.radius1, topy });
    rope.push_back({ centerx - 2.0f * kladkostroj.radius1, position1.y - kladkostroj.radius1 });
    rope.push_back({ centerx, position1.y - kladkostroj.radius1 });
    rope.push_back({ centerx, position2.y + kladkostroj.radius2 });
    rope.push_back({ centerx + 2.0f * kladkostroj.radius2, position2.y + kladkostroj.radius2 });
    rope.push_back({ weight2_position.x, weight2_position.y + weight2_height / 2.0f });

    kladkostroj.rope = world.CreateRope(rope, frame::col4::DARKGRAY, &kladkostroj.top, &kladkostroj.weight2);
    world.SetLayerRope(kladkostroj.rope, -10);
}

void setup()
{
    create_ground();

    //auto test = world.CreateRectangle({ 400.0f, 500.0f }, 60.0f, 40.0f);
    //world.SetStatic(test, true);
    //world.SetFill(test, frame::col4::RGB(60,60,60));

    //frame::vec2 anchorframe::vec2(400.0f, 480.0f);
    //frame::vec2 ropeSize(5.0f, 8.0f);
    //World::Object anchorObject = test;

    //std::vector<World::Object> objects;

    //for (int32_t i = 0; i < 50; i++)
    //{
    //    auto test2 = world.CreateRectangle({ anchorframe::vec2.x, anchorframe::vec2.y - ropeSize.y / 2.0f }, ropeSize.x, ropeSize.y);
    //    world.SetStatic(test2, false);
    //    world.SetFill(test2, frame::col4::RGB(60, 60, 60));
    //    world.CreateRevoluteJoint(anchorObject, test2, anchorframe::vec2);

    //    objects.push_back(test2);

    //    anchorframe::vec2 = frame::vec2(anchorframe::vec2.x, anchorframe::vec2.y - ropeSize.y);
    //    anchorObject = test2;
    //}

    //for (size_t i = 0; i < objects.size(); i += 5)
    //{
    //    frame::vec2 anchorDown = world.GetPosition(objects[i]) + frame::vec2(0.0f, ropeSize.y / 2.0f);
    //    for (size_t j = i + 1; j < objects.size(); j++)
    //    {
    //        frame::vec2 anchor = world.GetPosition(objects[j]) - frame::vec2(0.0f, ropeSize.y / 2.0f);
    //        world.CreateDistanceJoint(objects[i], objects[j], anchorDown, anchor, true);
    //    }

    //    frame::vec2 anchorUp = world.GetPosition(objects[i]) - frame::vec2(0.0f, ropeSize.y / 2.0f);
    //    for (int32_t j = i - 1; j >= 0; j--)
    //    {
    //        frame::vec2 anchor = world.GetPosition(objects[j]) + frame::vec2(0.0f, ropeSize.y / 2.0f);
    //        world.CreateDistanceJoint(objects[i], objects[j], anchorUp, anchor, true);
    //    }
    //}

    //auto testBody1 = world.CreateCircle({ 200.0f, 200.0f }, 20.0f);
    //world.SetStatic(testBody1, false);
    //auto testRope = world.CreateRope({ {100.0f, 100.0f}, {200.0f, 200.0f} }, nullptr, &testBody1);

    create_kladkostroj();

    frame::set_world_transform(frame::translation({ 0.0f, frame::get_screen_size().y }) * frame::scale({ 1.0f, -1.0f }));
}

void draw_gui()
{
    bool open = true;
    ImGui::SetNextWindowPos({ 20.0f, 20.0f });
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

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas");
    auto mouse_canvas = frame::get_world_transform().transform_point(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", frame::get_screen_size().x, frame::get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas Size");
    ImGui::Text("%.2f %.2f ", frame::get_world_size().x, frame::get_world_size().y);

    ImGui::EndMainMenuBar();
}

void handle_mouse()
{
    if (frame::is_mouse_pressed(frame::mouse_button::left))
    {
        if (rand() % 2)
            create_square();
        else
            create_circle();
    }

    if (frame::is_mouse_pressed(frame::mouse_button::right))
    {
        auto objects = world.QueryObjects(frame::get_mouse_world_position());
        if (!objects.empty())
            mouse_joint = world.CreateMouseJoint(objects[0], frame::get_mouse_world_position());
    }

    if (frame::is_mouse_released(frame::mouse_button::right))
    {
        if (mouse_joint)
            world.DestroyJoint(mouse_joint);
        mouse_joint = 0;
    }
}

void update()
{
    //draw_gui();
    draw_debug_gui();

    handle_mouse();

    world.Update();

    draw_kladkostroj();
}
