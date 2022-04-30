#pragma once

#include <nanovg.h>
#include "point.h"
#include "color.h"
#include <functional>

void setup();
void update();

extern NVGcontext* vg;

namespace frame
{
    // *** screen ***
    Point screen_to_gui(const Point& position);
    Point screen_to_canvas(const Point& position);
    // screen position is relative to lower-left corner (y goes up)
    void screen_background(const Color& color);
    float screen_width();
    float screen_height();
    using screen_resize_callback = std::function<void()>;
    int32_t screen_resize_register(screen_resize_callback callback);
    void screen_resize_unregister(int32_t handle);

    // *** canvas ***
    Point canvas_to_screen(const Point& position);
    Point canvas_to_gui(const Point& position);
    // y goes up (same as screen position)
    void canvas_background(const Color& color);
    // canvas screen position and size is in pixels, offset, scale and size below does not influence these
    float canvas_screen_width();
    float canvas_screen_height();
    Point canvas_screen_position();
    void canvas_set_screen_size(float width, float height);
    void canvas_set_screen_position(const Point& position);
    // value of lower-left point
    Point canvas_offset();
    Point canvas_scale();
    float canvas_width();
    float canvas_height();
    void canvas_set_offset(const Point& offset);
    // setting canvas size will change scale and vice-versa (screen size is not changed) 
    void canvas_set_scale(const Point& scale);
    void canvas_set_size(float width, float height);

    // *** gui *** 
    Point gui_to_screen(const Point& position);
    Point gui_to_canvas(const Point& position);

    // *** mouse ***
    using mouse_button_callback = std::function<void()>;
    enum class mouse_button { left, right, middle };
    Point mouse_screen_position();
    Point mouse_canvas_position();
    bool mouse_pressed(mouse_button button);
    // return change of mouse position from previous frame
    Point mouse_screen_delta();
    int32_t mouse_press_register(mouse_button button, mouse_button_callback callback);
    int32_t mouse_release_register(mouse_button button, mouse_button_callback callback);
    void mouse_unregister(int32_t handle);
    float mouse_wheel_delta();

    // *** drawing ***
    void draw_rectangle(const Point& position, float width, float height, const Color& color);
    void draw_rectangle_ex(const Point& position, 
                           float radians,
                           float width,
                           float height,
                           const Color& fill_color,
                           const float outline_thickness,
                           const Color& outline_color);
    void draw_rounded_rectangle(const Point& position, float width, float height, float radius, const Color& color);
    void draw_rounded_rectangle_ex(const Point& position, 
                                   float radians,
                                   float width,
                                   float height,
                                   float radius,
                                   const Color& fill_color,
                                   const float outline_thickness,
                                   const Color& outline_color);
    void draw_circle(const Point& position, float radius, const Color& color);
    void draw_circle_ex(const Point& position,
                        float radians,
                        float radius,
                        const Color& fill_color,
                        const float outline_thickness,
                        const Color& outline_color);
    //void draw_ellipse(const Point& position1, const Point& position2, float major, float minor, const Color& color);
    void draw_polygon(const Point& position, const Point* vertices, size_t count, const Color& color);
    void draw_polygon_ex(const Point& position,
                         float radians,
                         const Point* vertices,
                         size_t count,
                         const Color& fill_color,
                         const float outline_thickness,
                         const Color& outline_color);
    void draw_line_directed(const Point& from, const Point& to, const Color& color);
    void draw_line_directed_ex(const Point& from, const Point& to, float thickness, const Color& color);
    void draw_line_solid(const Point& from, const Point& to, const Color& color);
    void draw_line_solid_ex(const Point& from, const Point& to, float thickness, const Color& color);
    void draw_line_dashed(const Point& from, const Point& to, const Color& color);
    void draw_line_dashed_ex(const Point& from, const Point& to, float thickness, const Color& color);
    void draw_curve_quad(const Point& from, const Point& control, const Point& to, const Color& color);
    void draw_curve_quad_ex(const Point& from, const Point& control, const Point& to, float thickness, const Color& color);
    // position is number between 0 and 1 identifying point on line and offset is distance from line
    void draw_line_text(const Point& from, const Point& to, const char* text, float position, float size, float offset, Side side, const Color& color);
    void draw_curve(const std::vector<Point>& points, const Color& color);
    void draw_curve(const std::vector<Point>& points, float thickness, const Color& color);

    // *** text ***
    float text_width(const char* text, float size);
    float text_height(const char* text, float width, float size);
    Point text_dimensions(const char* text, float size);
    void text(const char* text, const Point& position, float width, float size, const Color& color);
    void text(const char* text, const Point& position, float size, const Color& color);

    // *** utils *** remove
    // Helper function to get point relative to width and height of screen.
    // x and y should be numbers between 0 and 1
    Point rel_pos(float x, float y);
    enum class Direction
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
    Point get_direction_vector(Direction direction);
}
