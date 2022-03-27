#include "framework.h"
#include "imgui.h"
#include "world.h"

int32_t circles_count = 0;
int32_t squares_count = 0;

World world;
World::Joint mouse_joint = 0;

void create_square()
{
    auto rect = world.CreateRectangle(frame::mouse_screen_position(), 10.0f, 10.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, nvgRGB(227, 142, 31));

    squares_count++;
}

void create_circle()
{
    auto rect = world.CreateCircle(frame::mouse_screen_position(), 5.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, nvgRGB(177, 242, 235));

    circles_count++;
}

void create_ground()
{
    auto ground = world.CreateRectangle(frame::rel_pos(0.5f, 0.0f), frame::screen_width(), 20.0f);
    world.SetStatic(ground, true);
    world.SetFill(ground, nvgRGB(80, 80, 80));
}

void setup()
{
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

    frame::canvas_background(nvgRGBf(0.1f, 0.1f, 0.1f));
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
    world.Draw();
}
