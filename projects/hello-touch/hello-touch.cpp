#include "framework.h"
#include "imgui.h"
#include "utils.h"

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
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

    auto touches = frame::get_touches_down();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Touches:");
    for (size_t i = 0; i < touches.size(); i++)
        ImGui::Text("%.2f %.2f", frame::get_touch_screen_position(touches[i]).x, frame::get_touch_screen_position(touches[i]).y);

    ImGui::End();
}

void setup()
{
    frame::set_screen_background(frame::col4::RGBf(0.1f, 0.1f, 0.1f));
    frame::set_world_transform(frame::translation({ 0.0f, frame::get_screen_size().y }) * frame::scale({ 1.0f, -1.0f }));
}

void update()
{
    frame::draw_coordinate_lines(frame::rgb(80, 80, 80));

    frame::draw_text("Hello, Touch!", { 100.0f, 100.0f }, 25.0f, frame::col4::RGBf(0.9f, 0.9f, 0.9f), frame::text_align::bottom_left);

    frame::draw_ellipse_ex({ 400.0f, 300.0f }, frame::deg_to_rad(60), 200.0f, 100.0f, frame::col4::BLANK, 1.0f / frame::get_world_scale().x, frame::col4::GOLD);
    frame::draw_circle({ 400.0f, 300.0f }, 100.0f, frame::rgb(0, 0, 200));

    draw_gui();
    draw_debug_gui();

    static frame::free_move_camera_config free_move_config;
    free_move_config.min_size = { 150.0f, 150.0f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 4000.0f, 4000.0f });

    frame::free_move_camera_update(free_move_config);

    if (frame::is_mouse_pressed(frame::mouse_button::left))
        frame::set_debug_touch_down(0, frame::get_mouse_screen_position());
    else if (frame::is_mouse_released(frame::mouse_button::left))
        frame::set_debug_touch_up(0);
    else if (frame::is_mouse_down(frame::mouse_button::left))
        frame::set_debug_touch_move(0, frame::get_mouse_screen_position());

    auto touches = frame::get_touches_down();

    for (size_t i = 0; i < touches.size(); i++)
    {
        for (size_t j = i + 1; j < touches.size(); j++)
        {
            auto from = frame::get_screen_to_world(frame::get_touch_screen_position(touches[i]));
            auto to = frame::get_screen_to_world(frame::get_touch_screen_position(touches[j]));
            frame::draw_line_solid(from, to, frame::col4::BLUE);
        }
    }

    for (const auto& t : touches)
        frame::draw_circle(frame::get_screen_to_world(frame::get_touch_screen_position(t)), 5.0f, frame::col4::RED);

    static frame::click_handler click(frame::mouse_button::left);
    click.update();

    static std::vector<frame::vec2> clicked_positions;
    if (click.is_clicked() && !frame::get_touches_released().empty())
    {
        clicked_positions.push_back(frame::get_screen_to_world(frame::get_touch_screen_position(frame::get_touches_released()[0])));
    }

    for (const auto& pos : clicked_positions)
        frame::draw_circle(pos, 5.0f, frame::col4::BLUE);
}
