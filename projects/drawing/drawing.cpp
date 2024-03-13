#include <framework.h>
#include <vector>

void setup()
{
    frame::set_world_transform(frame::translation({ 0.0f, frame::get_screen_size().y }) * frame::scale({ 1.0f, -1.0f }));
}

void update()
{
    frame::draw_rectangle({ 100.0f, 100.0f }, 50.0f, 30.0f, frame::col4::LIGHTGRAY);
    frame::draw_rectangle_ex({ 200.0f, 100.0f }, 0.3f, 20.0f, 80.0f, frame::col4::DARKGRAY, 5.0f, frame::col4::LIGHTGRAY);
    frame::draw_rounded_rectangle({ 300.0f, 100.0f }, 60.0f, 70.0f, 5.0f, frame::col4::ORANGE);
    frame::draw_rounded_rectangle_ex({ 400.0f, 100.0f }, 0.7f, 40.0f, 70.0f, 5.0f, frame::col4::BLANK, 3.0f, frame::col4::ORANGE);
    frame::draw_circle({ 500.0f, 100.0f }, 40.0f, frame::col4::GRAY);
    frame::draw_circle_ex({ 600.0f, 100.0f }, 1.0f, 30.0f, frame::col4::GRAY, 7.0f, frame::col4::LIGHTGRAY);
    std::vector<frame::vec2> polygon{ {0.0f, 0.0f}, {0.0f, 30.0f}, {15.0f, 40.0f}, {30.0f, 30.0f}, {30.0f, 0.0f} };
    frame::draw_polygon({ 700.0f, 100.0f }, polygon.data(), polygon.size(), frame::col4::LIGHTGRAY);
    frame::draw_polygon_ex({ 100.0f, 200.0f }, 0.7f, polygon.data(), polygon.size(), frame::col4::BLANK, 1.0f, frame::col4::LIGHTGRAY);
    frame::draw_line_solid({ 150.0f, 150.0f }, { 200.0f, 200.0f }, frame::col4::ORANGE);
    frame::draw_line_solid_ex({ 250.0f, 150.0f }, { 300.0f, 200.0f }, 4.0f, frame::col4::ORANGE);
    frame::draw_line_directed({ 350.0f, 150.0f }, { 400.0f, 200.0f }, frame::col4::ORANGE);
    frame::draw_line_directed_ex({ 450.0f, 150.0f }, { 500.0f, 200.0f }, 4.0f, frame::col4::ORANGE);
    frame::draw_line_dashed({ 550.0f, 150.0f }, { 600.0f, 200.0f }, frame::col4::ORANGE);
    frame::draw_line_dashed_ex({ 650.0f, 150.0f }, { 700.0f, 200.0f }, 4.0f, frame::col4::ORANGE);
    frame::draw_quad_bezier({ 150.0f, 250.0f }, { 200.0f, 300.0f }, { 250.0f, 250.0f }, frame::col4::DARKGRAY);
    frame::draw_quad_bezier_ex({ 250.0f, 250.0f }, { 300.0f, 400.0f }, { 350.0f, 250.0f }, 4.0f, frame::col4::DARKGRAY);
    frame::draw_line_solid({ 350.0f, 250.0f }, { 450.0f, 300.0f }, frame::col4::ORANGE);
}
