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
    world.SetFill(rect, Color::RGB(227, 142, 31));

    squares_count++;
}

void create_circle()
{
    auto rect = world.CreateCircle(frame::mouse_canvas_position(), 5.0f);
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

void show_resize_type_combo()
{
    static int index = 0;
    ImGui::Combo("##", &index, "None\0ResizeWorld\0ResizeScreen\0");

    resize_type = (ResizeType)index;
}

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x(), mouse_screen.y());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas");
    auto mouse_canvas = frame::screen_to_canvas(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x(), mouse_canvas.y());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", frame::screen_width(), frame::screen_height());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Canvas Size");
    ImGui::Text("%.2f %.2f ", frame::canvas_width(), frame::canvas_height());

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

void apply_resize_none(float width, float height)
{
    // move canvas to center of screen
    Point canvas_position((width - frame::canvas_width()) / 2.0f, (height - frame::canvas_height()) / 2.0f);
    frame::canvas_set_screen_size(800.0f, 600.0f);
    frame::canvas_set_screen_position(canvas_position);
    frame::canvas_set_size(800.0f, 600.0f);
}

void apply_resize_screen(float width, float height)
{
    frame::canvas_set_screen_size(width, height);
    frame::canvas_set_screen_position({ 0.0f, 0.0f });
    frame::canvas_set_size(800.0f, 600.0f);
}

void apply_resize_world(float width, float height)
{
    frame::canvas_set_screen_size(width, height);
    frame::canvas_set_screen_position({ 0.0f, 0.0f });
    frame::canvas_set_size(width, height);
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

    frame::screen_resize_register([]()
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

    frame::canvas_background(Color::RGBf(0.1f, 0.1f, 0.1f));
}

void update()
{
    draw_gui();
    draw_debug_gui();

    //auto size = frame::text_dimensions("RESIZE TEST", 60.0f);
    //frame::rectangle(frame::rel_pos(0.1f, 0.6f) + Point{size.x() / 2.0f, size.y() / 2.0f}, 0.0f, size.x(), size.y(), nvgRGBf(0.0f, 0.0f, 0.0f));
    frame::text("RESIZE TEST", frame::rel_pos(0.1f, 0.6f), 60.0f, Color::RGBf(0.3f, 0.3f, 0.3f));

    world.Update();
    world.Draw();
}
