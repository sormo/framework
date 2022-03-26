#include "framework.h"
#include "imgui.h"
#include "world.h"

int32_t circles_count = 0;
int32_t squares_count = 0;

enum class ResizeType
{
    None = 0,
    ResizeWorld,
    ResizeScreen

} resize_type;

World world;

void create_square()
{
    auto rect = world.CreateRectangle(frame::mouse_canvas_position(), 10.0f, 10.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, nvgRGB(227, 142, 31));

    squares_count++;
}

void create_circle()
{
    auto rect = world.CreateCircle(frame::mouse_canvas_position(), 5.0f);
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

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x(), mouse_screen.y());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas");
    auto mouse_canvas = frame::convert_from_screen_to_canvas(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x(), mouse_canvas.y());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", frame::screen_width(), frame::screen_height());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas Size");
    ImGui::Text("%.2f %.2f", frame::canvas_width(), frame::canvas_height());

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

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas Offset");
    ImGui::SameLine();
    ImGui::Text("%.2f %.2f", frame::canvas_offset().x(), frame::canvas_offset().y());
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Use right mouse button to move canvas offset");

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas Scale");
    ImGui::SameLine();
    ImGui::Text("%.2f %.2f", frame::canvas_scale().x(), frame::canvas_scale().y());
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

void modify_canvas_offset()
{
    if (frame::mouse_pressed(frame::mouse_button::right))
    {
        frame::set_canvas_offset(frame::canvas_offset() + frame::mouse_screen_delta());
    }
}

void modify_canvas_scale()
{
    if (frame::mouse_wheel_delta())
    {
        float new_scale = frame::canvas_scale().x() + frame::mouse_wheel_delta() * 0.01f;
        if (new_scale < 0.1f)
            new_scale = 0.1f;

        // preserver canvas mouse position after scale
        auto canvas_mouse_position_before = frame::mouse_canvas_position();
        
        frame::set_canvas_scale({ new_scale, new_scale });
        
        auto canvas_mouse_position_after = frame::mouse_canvas_position();
        auto canvas_delta = canvas_mouse_position_before - canvas_mouse_position_after;

        // TODO is bug that we need to multiply by scale ?
        frame::set_canvas_offset(frame::canvas_offset() + canvas_delta * frame::canvas_scale());
    }
}

void setup()
{
    create_ground();

    frame::mouse_press_register(frame::mouse_button::left, create_world_object);

    frame::canvas_background(nvgRGBf(0.1f, 0.1f, 0.1f));
}

void update()
{
    draw_gui();
    draw_debug_gui();
    modify_canvas_offset();
    modify_canvas_scale();

    frame::text("CAMERA TEST", {10.0f, 300.0f}, 60.0f, nvgRGBf(0.3f, 0.3f, 0.3f));

    world.Update();
    world.Draw();
}
