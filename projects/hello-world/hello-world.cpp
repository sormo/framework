#include <framework.h>
#include "imgui.h"

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_canvas = frame::get_world_transform().inverted().transform_point(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", frame::get_screen_size().x, frame::get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", frame::get_world_size().x, frame::get_world_size().y);

    ImGui::EndMainMenuBar();
}

void setup()
{
    auto size = frame::get_screen_size();

    frame::set_world_transform(frame::translation(size / 2.0f) * frame::scale({ 1.5f, 1.5f }));
}

void update()
{
    draw_debug_gui();

    frame::draw_circle({ 0.0f, 0.0f }, 20.0f, frame::rgba(0, 0, 255, 128));

    auto rect = frame::get_text_rectangle("Hello, World!", { -100.0f, 100.0f }, 25.0f, frame::text_align::bottom_left);

    frame::draw_rectangle(rect.center(), rect.size().x, rect.size().y, frame::rgb(123, 12, 12));
    frame::draw_text("Hello, World!", { -100.0f, 100.0f }, 25.0f, frame::col4::RGBf(0.9f, 0.9f, 0.9f), frame::text_align::bottom_left);
}
