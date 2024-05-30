#include "framework.h"
#include "imgui.h"
#include "world.h"
#include "utils.h"

int32_t circles_count = 0;
int32_t squares_count = 0;

World world;

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

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_world = frame::get_mouse_world_position();
    ImGui::Text("%.3f %.3f", mouse_world.x, mouse_world.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", frame::get_screen_size().x, frame::get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f", frame::get_world_size().x, frame::get_world_size().y);

    ImGui::EndMainMenuBar();
}

void draw_gui()
{
    bool open = true;
    ImGui::SetNextWindowPos({ 0.0f, 20.0f });
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Circles");
    ImGui::SameLine();
    ImGui::Text("%d", circles_count);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Squares");
    ImGui::SameLine();
    ImGui::Text("%d", squares_count);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mouse Delta");
    ImGui::Text("%.2f %.2f", frame::get_mouse_screen_delta().x, frame::get_mouse_screen_delta().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Wheel Delta");
    ImGui::Text("%.2f", frame::get_mouse_wheel_delta());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Offset");
    ImGui::SameLine();
    auto translation = frame::get_world_translation();
    ImGui::Text("%.2f %.2f", translation.x, translation.y);
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Use right mouse button to move world offset");

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Scale");
    ImGui::SameLine();
    auto scale = frame::get_world_scale();
    ImGui::Text("%.2f %.2f", scale.x, scale.y);
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Use mouse wheel to modify scale");

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void create_world_object()
{
    if (rand() % 2)
        create_square();
    else
        create_circle();
}

void setup()
{
    create_ground();

    frame::set_screen_background(frame::col4::RGBf(0.1f, 0.1f, 0.1f));
    frame::set_world_transform(frame::translation({ 0.0f, frame::get_screen_size().y }) * frame::scale({ 1.0f, -1.0f }));
}

void update()
{
    frame::draw_coordinate_lines(frame::rgb(80, 80, 80));
    world.Draw();

    frame::draw_text("Hello, World!", { 100.0f, 100.0f }, 25.0f, frame::col4::RGBf(0.9f, 0.9f, 0.9f), frame::text_align::bottom_left);

    frame::draw_ellipse_ex({ 400.0f, 300.0f }, frame::deg_to_rad(60), 200.0f, 100.0f, frame::col4::BLANK, 1.0f / frame::get_world_scale().x, frame::col4::GOLD);
    frame::draw_circle({ 400.0f, 300.0f }, 100.0f, frame::rgb(0, 0, 200));

    draw_gui();
    draw_debug_gui();

    static frame::free_move_camera_config free_move_config;
    free_move_config.min_size = { 150.0f, 150.0f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 4000.0f, 4000.0f });

    frame::free_move_camera_update(free_move_config);

    if (frame::is_mouse_pressed(frame::mouse_button::left))
        create_world_object();

    world.Update();
}
