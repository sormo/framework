#include "framework.h"
#include "imgui.h"
#include "world.h"

constexpr float MinGuiWidth = 300.0f;
constexpr float MinGuiHeight = 120.0f;

Point guiPosition;
Point guiSize;

int32_t numberOfCircles = 10;
int32_t numberOfSquares = 50;

int32_t circlesCount = 0;
int32_t squaresCount = 0;

World world;

void setupCanvas()
{
    float size = frame::screen_width() > frame::screen_height() ? frame::screen_height() : frame::screen_width();

    if (frame::screen_width() > frame::screen_height())
    {
        float remainingWidth = frame::screen_width() - size;
        if (remainingWidth > MinGuiWidth)
        {
            frame::set_canvas_position({ frame::screen_width() - size, 0.0f });
            guiPosition = { 0.0f, 0.0f };
            guiSize = { remainingWidth, frame::screen_height() };
        }
        else
        {
            size -= MinGuiHeight;
            frame::set_canvas_position({ (frame::screen_width() - size) / 2.0f, 0.0f });
            guiPosition = { (frame::screen_width() - size) / 2.0f, 0.0f };
            guiSize = { size, MinGuiHeight };
        }
    }
    else
    {
        float remainingHeight = frame::screen_height() - size;
        if (remainingHeight > MinGuiHeight)
        {
            frame::set_canvas_position({ 0.0f, 0.0f });
            guiPosition = { 0.0f, 0.0f };
            guiSize = { size, frame::screen_height() - size };
        }
        else
        {
            size = frame::screen_height() - MinGuiHeight;
            frame::set_canvas_position({ (frame::screen_width() - size) / 2.0f, 0.0f });
            guiPosition = { (frame::screen_width() - size) / 2.0f, 0.0f };
            guiSize = { size, MinGuiHeight };
        }
    }

    frame::set_canvas_size(size, size);
}

void createSquares(int32_t count)
{
    auto position = frame::rel_pos(0.5f, 0.5f);

    for (auto i = 0; i < count; i++)
    {
        auto rect = world.CreateRectangle(position, 10.0f, 10.0f);
        world.SetStatic(rect, false);
        world.SetFill(rect, nvgRGB(227, 142, 31));
    }
}

void createCircles(int32_t count)
{
    auto position = frame::rel_pos(0.5f, 0.5f);

    for (auto i = 0; i < numberOfCircles; i++)
    {
        auto rect = world.CreateCircle(position, 5.0f);
        world.SetStatic(rect, false);
        world.SetFill(rect, nvgRGB(177, 242, 235));
    }
}

void recreateWorld()
{
    world.Clear();

    // TODO use similar pattern as in text manager ???
    auto ground = world.CreateRectangle(frame::rel_pos(0.5f, 0.0f), frame::screen_width(), 20.0f);
    world.SetStatic(ground, true);
    world.SetFill(ground, nvgRGB(80, 80, 80));

    createCircles(circlesCount);
    createSquares(squaresCount);
}

void createObjects()
{
    // TODO rel pos for canvas ???

    createSquares(numberOfSquares);
    squaresCount += numberOfSquares;

    createCircles(numberOfCircles);
    circlesCount += numberOfCircles;
}

void setup()
{
    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;

    setupCanvas();

    recreateWorld();

    frame::resize_register([](float, float)
    {
        setupCanvas();
        recreateWorld();
    });

    frame::mouse_press_register([]()
    {
        createObjects();
    });

    frame::canvas_background(nvgRGBf(0.1f, 0.1f, 0.1f));
}

void drawGui()
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

    ImGui::SetNextWindowPos({ guiPosition.x(), guiPosition.y() });
    ImGui::SetNextWindowSize({ guiSize.x(), guiSize.y() });

    bool open = true;
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::SliderInt(u8"Kruhy", &numberOfCircles, 0, 30);
    ImGui::SameLine();
    ImGui::Text("%d", circlesCount);

    ImGui::SliderInt(u8"Štvorce", &numberOfSquares, 0, 100);
    ImGui::SameLine();
    ImGui::Text("%d", squaresCount);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    auto frameSize = ImGui::GetContentRegionAvail();
    if (frameSize.y > frameSize.x)
        frameSize.y = frameSize.x;

    ImGui::PlotLines("", FrameTimes, FrameCount, FrameIndex, nullptr, 0.0f, 60.0f, frameSize);

    ImGui::End();

    //ImGui::ShowDemoWindow();
}

void update()
{
    drawGui();
    world.Update();
    world.Draw();
}
