#include "framework.h"
#include "imgui.h"
#include "world.h"

int32_t circles_count = 0;
int32_t squares_count = 0;

World world;
World::Joint mouse_joint = 0;

bool create_on_hold = false;

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

void setup()
{
    create_ground();

    frame::set_screen_background(frame::col4::RGBf(0.1f, 0.1f, 0.1f));
    frame::set_world_transform(frame::translation({ 0.0f, frame::get_screen_size().y }) * frame::scale({ 1.0f, -1.0f }));
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

    ImGui::Checkbox("Create while holding button", &create_on_hold);

    ImGui::End();
}

void update()
{
    draw_gui();

    handle_mouse();

    if (create_on_hold && frame::is_mouse_down(frame::mouse_button::left))
    {
        if (rand() % 2)
            create_square();
        else
            create_circle();
    }

    world.Update();
    world.Draw();
}
