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

    if (LineRotationRunning)
    {
        LineAngle += RotationSpeed;
        if (LineAngle > 2.0f * PI)
            LineAngle -= 2.0f * PI;
    }

    LineAngleTest = (LinePoint2 - LinePoint1).Angle();

    draw_gui();
}
