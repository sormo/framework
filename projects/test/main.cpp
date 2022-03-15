#include <framework.h>
#include <iostream>
#include "world.h"
#include "imgui.h"

World world;

float LineAngle = 0.0f;
float LineAngleTest = 0.0f;
Point LinePoint1;
Point LinePoint2;

float RotationSpeed = 0.01f;
bool LineRotationRunning = true;

float LinePointRow = 0.5f;

float TextOffset = 0.0f;

constexpr float PI = 3.141592654f;
constexpr float LineLenght = 200.0f;

void setup_test_world()
{
    for (int i = 0; i < 200; ++i)
    {
        auto obj = world.CreateRectangle(frame::rel_pos(0.5f, 0.8f), 10.0f, 10.0f);
        world.SetStatic(obj, false);
    }

    for (int i = 0; i < 50; ++i)
    {
        auto obj = world.CreateCircle(frame::rel_pos(0.5f, 0.8f), 5.0f);
        world.SetStatic(obj, false);
        world.SetFill(obj, nvgRGB(255, 128, 64));
    }

    auto obj2 = world.CreateRectangle(frame::rel_pos(0.5f, 0.8f), 50.0f, 50.0f);
    world.SetStatic(obj2, false);

    auto obj3 = world.CreateRectangle(frame::rel_pos(0.5f, 0.1f), frame::screen_width(), 50.0f);
    world.SetFill(obj3, nvgRGB(128, 128, 128));
}

void setup()
{
    std::cout << "Hello!\n";
    std::cout << "width: " << frame::screen_width() << " height: " << frame::screen_height() << "\n";

    frame::screen_background(nvgRGBf(0.1f, 0.1f, 0.1f));

    LinePoint1 = frame::rel_pos(0.5f, 0.5f);

    frame::mouse_press_register([]()
    {
        LineRotationRunning = !LineRotationRunning;
    });

    //setup_test_world();
}

// return number between 0 to 1 of current eighth
float get_angle_eighth(float radians)
{
    return std::fmodf(radians, PI / 4.0f) / (PI / 4.0f);
}

void draw_gui()
{
    ImGui::BeginMainMenuBar();
    
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Point");
    ImGui::Text("%.3f %.3f", LinePoint2.x() - LinePoint1.x(), LinePoint2.y() - LinePoint1.y());

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Angle");
    ImGui::Text("%.3f", LineAngle);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Angle Test");
    ImGui::Text("%.3f", LineAngleTest);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Eighth");
    ImGui::Text("%.3f", get_angle_eighth(LineAngle));

    ImGui::EndMainMenuBar();

    //

    ImGui::Begin("Settings");
    ImGui::SliderFloat("Line Point Row", &LinePointRow, 0.0f, 1.0f);
    ImGui::SliderFloat("Speed", &RotationSpeed, 0.001f, 0.1f);
    ImGui::SliderFloat("Offset", &TextOffset, 0.0f, 50.0f);
    ImGui::End();
}

enum class QuadrantDirection
{
    Right,
    UpRight,
    Up,
    UpLeft,
    Left,
    DownLeft,
    Down,
    DownRight
};

QuadrantDirection get_start(float radians)
{
    if (radians < 1.0f * PI / 4.0f)
        return QuadrantDirection::Right;
    else if (radians < 2.0f * PI / 4.0f)
        return QuadrantDirection::UpRight;
    else if (radians < 3.0f * PI / 4.0f)
        return QuadrantDirection::Up;
    else if (radians < 4.0f * PI / 4.0f)
        return QuadrantDirection::UpLeft;
    else if (radians < 5.0f * PI / 4.0f)
        return QuadrantDirection::Left;
    else if (radians < 6.0f * PI / 4.0f)
        return QuadrantDirection::DownLeft;
    else if (radians < 7.0f * PI / 4.0f)
        return QuadrantDirection::Down;
    else if (radians < 8.0f * PI / 4.0f)
        return QuadrantDirection::DownRight;
}

QuadrantDirection get_end(float radians)
{
    radians += PI / 4.0f;
    if (radians > 2.0f * PI)
        radians -= 2.0f * PI;

    return get_start(radians);
}

// return point on text rectangle which should match with line on point
Point get_rect_point(QuadrantDirection direction)
{
    switch (direction)
    {
    case QuadrantDirection::Right:
        return { -0.5f,  0.0f };
    case QuadrantDirection::UpRight:
        return { -1.0f,  0.0f };
    case QuadrantDirection::Up:
        return { -1.0f, -0.5f };
    case QuadrantDirection::UpLeft:
        return { -1.0f, -1.0f };
    case QuadrantDirection::Left:
        return { -0.5f, -1.0f };
    case QuadrantDirection::DownLeft:
        return { 0.0f, -1.0f };
    case QuadrantDirection::Down:
        return { 0.0f, -0.5f };
    case QuadrantDirection::DownRight:
        return { 0.0f,  0.0f };
    }
}

void update()
{
    auto lineVector = Point(1.0f, 0.0f).Rotated(LineAngle);

    LinePoint2 = LinePoint1 + lineVector * LineLenght;
    //frame::line_solid(LinePoint1, LinePoint2, nvgRGB(255, 255, 255));

    auto linePoint = LinePoint1 + lineVector * LineLenght * LinePointRow;
    frame::circle(linePoint, 0.0f, 2.0f, nvgRGB(255, 0, 0));
    //auto horizontalLine = Point(300.0f, 0.0f);
    //frame::line_solid(linePoint - horizontalLine, linePoint + horizontalLine, nvgRGB(50, 50, 50));

    frame::line_directed(LinePoint1, LinePoint2, nvgRGB(255, 255, 255));
    frame::line_text(LinePoint1, LinePoint2, "AHOJ", LinePointRow, 40.0f, TextOffset, frame::Side::Left, nvgRGB(222, 150, 80));
    //frame::line_text(LinePoint1, LinePoint2, "g", 1.0f - LinePointRow, 15.0f, frame::Side::Right, nvgRGB(222, 150, 80));

    //
    //const char text[] = "AHOJ";
    //Point rectSize = frame::text_dimensions(text, 60.0f);
    //{
    //    Point rectStart = linePoint;
    //    rectStart = rectStart + rectSize * get_rect_point(get_start(LineAngle));
    //    rectStart = rectStart + rectSize / 2.0f;

    //    Point rectEnd = linePoint;
    //    rectEnd = rectEnd + rectSize * get_rect_point(get_end(LineAngle));
    //    rectEnd = rectEnd + rectSize / 2.0f;

    //    Point rectPosition = rectStart.Interpolate(rectEnd, get_angle_eighth(LineAngle));
    //    frame::rectangle(rectPosition, 0.0f, rectSize.x(), rectSize.y(), nvgRGBA(50, 80, 60, 120));
    //    frame::text(text, rectPosition - rectSize / 2.0f, 60.0f, nvgRGB(255, 255, 255));
    //}
    //// ---
    //{
    //    Point rectStart = linePoint;
    //    rectStart = rectStart - rectSize * get_rect_point(get_start(LineAngle));
    //    rectStart = rectStart - rectSize / 2.0f;

    //    Point rectEnd = linePoint;
    //    rectEnd = rectEnd - rectSize * get_rect_point(get_end(LineAngle));
    //    rectEnd = rectEnd - rectSize / 2.0f;

    //    Point rectPosition = rectStart.Interpolate(rectEnd, get_angle_eighth(LineAngle));
    //    frame::rectangle(rectPosition, 0.0f, rectSize.x(), rectSize.y(), nvgRGBA(50, 80, 60, 120));
    //    frame::text(text, rectPosition - rectSize / 2.0f, 60.0f, nvgRGB(255, 255, 255));
    //}
    //

    if (LineRotationRunning)
    {
        LineAngle += RotationSpeed;
        if (LineAngle > 2.0f * PI)
            LineAngle -= 2.0f * PI;
    }

    LineAngleTest = (LinePoint2 - LinePoint1).Angle();

    draw_gui();
}
