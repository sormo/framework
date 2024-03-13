#include "framework.h"
#include "imgui.h"
#include "world.h"

using namespace frame;

int32_t circles_count = 0;
int32_t squares_count = 0;

vec2 original_screen_size;

enum class ResizeType
{
    ResizeWorld = 0,
    ResizeScreen

} resize_type;

World world;

void create_square()
{
    auto rect = world.CreateRectangle(get_mouse_world_position(), 10.0f, 10.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, frame::col4::RGB(227, 142, 31));

    squares_count++;
}

void create_circle()
{
    auto rect = world.CreateCircle(get_mouse_world_position(), 5.0f);
    world.SetStatic(rect, false);
    world.SetFill(rect, frame::col4::RGB(177, 242, 235));

    circles_count++;
}

void create_ground()
{
    auto ground = world.CreateRectangle(get_world_position_screen_relative({ 0.5f, 0.0f }), get_screen_size().x, 20.0f);
    world.SetStatic(ground, true);
    world.SetFill(ground, frame::col4::RGB(80, 80, 80));
}

void show_resize_type_combo()
{
    static int index = 0;
    ImGui::Combo("##", &index, "ResizeWorld\0ResizeScreen\0");

    resize_type = (ResizeType)index;
}

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_canvas = get_world_transform().inverted().transform_point(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", get_screen_size().x, get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", get_world_size().x, get_world_size().y);

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

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Resize");
    ImGui::SameLine();
    show_resize_type_combo();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void apply_resize_screen()
{
    set_world_transform(translation({ 0.0f, get_screen_size().y }) * scale({ 1.0f, -1.0f }));
}

void apply_resize_world()
{
    vec2 s = get_screen_size() / original_screen_size;
    set_world_transform(translation({ 0.0f, get_screen_size().y }) * scale({ s.x, -s.y }));
}

void setup()
{
    create_ground();

    original_screen_size = get_screen_size();

    set_screen_background(frame::col4::RGBf(0.1f, 0.1f, 0.1f));

    set_world_transform(translation({ 0.0f, get_screen_size().y }) * scale({ 1.0f, -1.0f }));

    auto m = mat3::translation({ 0.0f, 600.0f }) * mat3::scaling({ 1.0f, -1.0f });
    auto mi = m.inverted();
    auto p = mi.transform_point(vec2{ 0.0f, 0.0f });

    int i = 0;
}

void update()
{
    draw_gui();
    draw_debug_gui();

    if (is_mouse_pressed(mouse_button::left))
    {
        if (rand() % 2)
            create_square();
        else
            create_circle();
    }

    if (is_screen_resized())
    {
        switch (resize_type)
        {
        case ResizeType::ResizeScreen:
            apply_resize_screen();
            break;
        case ResizeType::ResizeWorld:
            apply_resize_world();
            break;
        }
    }

    auto text_rect = get_text_rectangle("RESIZE TEST", {}, 60.0f);
    draw_rectangle(get_world_position_screen_relative({ 0.1f, 0.6f }) + vec2{ text_rect.size().x / 2.0f, text_rect.size().y / 2.0f }, text_rect.size().x, text_rect.size().y, rgb(0,0,0));
    draw_text("RESIZE TEST", get_world_position_screen_relative({ 0.1f, 0.6f }), 60.0f, frame::col4::RGBf(0.3f, 0.3f, 0.3f));

    world.Update();
    world.Draw();
}
