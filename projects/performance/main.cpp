#include "framework.h"
#include "imgui.h"
#include "world.h"

constexpr float MinGuiWidth = 300.0f;
constexpr float MinGuiHeight = 120.0f;

Point gui_position;
Point gui_size;

const int32_t number_circles = 10;
const int32_t number_squares = 50;

int32_t circles_count = 0;
int32_t squares_count = 0;

World world;

void setup_canvas()
{
    float size = frame::screen_width() > frame::screen_height() ? frame::screen_height() : frame::screen_width();

    if (frame::screen_width() > frame::screen_height())
    {
        float remaining_width = frame::screen_width() - size;
        if (remaining_width > MinGuiWidth)
        {
            frame::set_canvas_position({ frame::screen_width() - size, 0.0f });
            gui_position = { 0.0f, 0.0f };
            gui_size = { remaining_width, frame::screen_height() };
        }
        else
        {
            size -= MinGuiHeight;
            frame::set_canvas_position({ (frame::screen_width() - size) / 2.0f, 0.0f });
            gui_position = { (frame::screen_width() - size) / 2.0f, 0.0f };
            gui_size = { size, MinGuiHeight };
        }
    }
    else
    {
        float remaining_height = frame::screen_height() - size;
        if (remaining_height > MinGuiHeight)
        {
            frame::set_canvas_position({ 0.0f, 0.0f });
            gui_position = { 0.0f, 0.0f };
            gui_size = { size, frame::screen_height() - size };
        }
        else
        {
            size = frame::screen_height() - MinGuiHeight;
            frame::set_canvas_position({ (frame::screen_width() - size) / 2.0f, 0.0f });
            gui_position = { (frame::screen_width() - size) / 2.0f, 0.0f };
            gui_size = { size, MinGuiHeight };
        }
    }

    frame::set_canvas_size(size, size);
}

void create_squares(int32_t count)
{
    auto position = frame::rel_pos(0.5f, 0.5f);

    for (auto i = 0; i < count; i++)
    {
        auto rect = world.CreateRectangle(position, 10.0f, 10.0f);
        world.SetStatic(rect, false);
        world.SetFill(rect, nvgRGB(227, 142, 31));
    }
}

void create_circles(int32_t count)
{
    auto position = frame::rel_pos(0.5f, 0.5f);

    for (auto i = 0; i < count; i++)
    {
        auto rect = world.CreateCircle(position, 5.0f);
        world.SetStatic(rect, false);
        world.SetFill(rect, nvgRGB(177, 242, 235));
    }
}

void recreate_world()
{
    world.Clear();

    // TODO use similar pattern as in text manager ???
    auto ground = world.CreateRectangle(frame::rel_pos(0.5f, 0.0f), frame::screen_width(), 20.0f);
    world.SetStatic(ground, true);
    world.SetFill(ground, nvgRGB(80, 80, 80));

    create_circles(circles_count);
    create_squares(squares_count);
}

void create_objects()
{
    // TODO rel pos for canvas ???

    create_squares(number_squares);
    squares_count += number_squares;

    create_circles(number_circles);
    circles_count += number_circles;
}

void setup()
{
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;

    setup_canvas();

    recreate_world();

    frame::resize_register([]()
    {
        setup_canvas();
        recreate_world();
    });

    frame::mouse_press_register([]()
    {
        create_objects();
    });

    frame::canvas_background(nvgRGBf(0.1f, 0.1f, 0.1f));
}

void draw_gui()
{
    constexpr int32_t FrameCount = 300;
    constexpr int32_t FrameSample = 20;

    static float FrameTimes[FrameCount] = {};
    static int32_t FrameIndex = 0;
    static int32_t FrameSampleCounter = 0;

    if (++FrameSampleCounter >= FrameSample)
    {
        FrameTimes[FrameIndex] = ImGui::GetIO().Framerate;
        FrameIndex++;
        if (FrameIndex >= FrameCount)
            FrameIndex = 0;

        FrameSampleCounter = 0;
    }

    ImGui::SetNextWindowPos({ gui_position.x(), gui_position.y() });
    ImGui::SetNextWindowSize({ gui_size.x(), gui_size.y() });

    bool open = true;
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::Text("Circles %d", circles_count);
    ImGui::Text("Squares %d", squares_count);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    auto frame_size = ImGui::GetContentRegionAvail();
    if (frame_size.y > frame_size.x)
        frame_size.y = frame_size.x;

    ImGui::PlotLines("", FrameTimes, FrameCount, FrameIndex, nullptr, 0.0f, 60.0f, frame_size);

    ImGui::End();

    //ImGui::ShowDemoWindow();
}

void update()
{
    draw_gui();
    world.Update();
    world.Draw();
}
