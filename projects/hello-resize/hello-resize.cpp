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

void show_resize_type_combo()
{
    static int index = 0;
    ImGui::Combo("", &index, "None\0ResizeWorld\0ResizeScreen\0");

    resize_type = (ResizeType)index;
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

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Resize");
    ImGui::SameLine();
    show_resize_type_combo();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void apply_resize_none(float width, float height)
{
    // move canvas to center of screen
    Point canvas_position((width - frame::canvas_width()) / 2.0f, (height - frame::canvas_height()) / 2.0f);
    frame::set_canvas_position(canvas_position);
}

void apply_resize_screen(float, float)
{
    // TODO
}

void apply_resize_world(float width, float height)
{
    frame::set_canvas_size(width, height);
    frame::set_canvas_position({ 0.0f, 0.0f });
}

void setup()
{
    create_ground();

    frame::mouse_press_register([]()
    {
        if (rand() % 2)
            create_square();
        else
            create_circle();
    });

    frame::resize_register([]()
    {
        switch (resize_type)
        {
        case ResizeType::None:
            apply_resize_none(frame::screen_width(), frame::screen_height());
            break;
        case ResizeType::ResizeScreen:
            apply_resize_screen(frame::screen_width(), frame::screen_height());
            break;
        case ResizeType::ResizeWorld:
            apply_resize_world(frame::screen_width(), frame::screen_height());
            break;
        }
    });

    frame::canvas_background(nvgRGBf(0.1f, 0.1f, 0.1f));
}

void update()
{
    draw_gui();

    world.Update();
    world.Draw();
}
