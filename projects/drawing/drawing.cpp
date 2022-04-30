#include <framework.h>
#include <vector>

void setup()
{
    
}

void update()
{
    frame::draw_rectangle({ 100.0f, 100.0f }, 50.0f, 30.0f, Color::LIGHTGRAY);
    frame::draw_rectangle_ex({ 200.0f, 100.0f }, 0.3f, 20.0f, 80.0f, Color::DARKGRAY, 5.0f, Color::LIGHTGRAY);
    frame::draw_rounded_rectangle({300.0f, 100.0f}, 60.0f, 70.0f, 5.0f, Color::ORANGE);
    frame::draw_rounded_rectangle_ex({400.0f, 100.0f}, 0.7f, 40.0f, 70.0f, 5.0f, Color::BLANK, 3.0f, Color::ORANGE);
    frame::draw_circle({ 500.0f, 100.0f }, 40.0f, Color::GRAY);
    frame::draw_circle_ex({ 600.0f, 100.0f }, 1.0f, 30.0f, Color::GRAY, 7.0f, Color::LIGHTGRAY);
    std::vector<Point> polygon{ {0.0f, 0.0f}, {0.0f, 30.0f}, {15.0f, 40.0f}, {30.0f, 30.0f}, {30.0f, 0.0f}};
    frame::draw_polygon({ 700.0f, 100.0f }, polygon.data(), polygon.size(), Color::LIGHTGRAY);
    frame::draw_polygon_ex({ 100.0f, 200.0f }, 0.7f, polygon.data(), polygon.size(), Color::BLANK, 1.0f, Color::LIGHTGRAY);
    frame::draw_line_solid({ 150.0f, 150.0f }, { 200.0f, 200.0f }, Color::ORANGE);
    frame::draw_line_solid_ex({ 250.0f, 150.0f }, { 300.0f, 200.0f }, 4.0f, Color::ORANGE);
    frame::draw_line_directed({ 350.0f, 150.0f }, { 400.0f, 200.0f }, Color::ORANGE);
    frame::draw_line_directed_ex({ 450.0f, 150.0f }, { 500.0f, 200.0f }, 4.0f, Color::ORANGE);
    frame::draw_line_dashed({ 550.0f, 150.0f }, { 600.0f, 200.0f }, Color::ORANGE);
    frame::draw_line_dashed_ex({ 650.0f, 150.0f }, { 700.0f, 200.0f }, 4.0f, Color::ORANGE);
    frame::draw_curve_quad({ 150.0f, 250.0f }, {200.0f, 300.0f}, { 250.0f, 250.0f }, Color::DARKGRAY);
    frame::draw_curve_quad_ex({ 250.0f, 250.0f }, { 300.0f, 400.0f }, { 350.0f, 250.0f }, 4.0f, Color::DARKGRAY);
    frame::draw_line_solid({ 350.0f, 250.0f }, { 450.0f, 300.0f }, Color::ORANGE);
    frame::draw_line_text({ 350.0f, 250.0f }, { 450.0f, 300.0f }, "test", 0.5f, 12.0f, 3.0f, frame::Side::Left, Color::ORANGE);
}
