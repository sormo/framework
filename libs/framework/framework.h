#pragma once

#include <nanovg.h>
#include "point.h"
#include <functional>

void setup();
void update();

extern NVGcontext* vg;

namespace frame
{
    // Helper function to get point relative to width and height of screen.
    // x and y should be numbers between 0 and 1
    Point rel_pos(float x, float y);

    Point convert_from_gui_to_screen(const Point& position);
    Point convert_from_canvas_to_screen(const Point& position);
    Point convert_from_screen_to_gui(const Point& position);
    Point convert_from_screen_to_canvas(const Point& position);

    // *** screen ***
    // screen position is relative to lower-left corner (y goes up)
    void screen_background(const NVGcolor& color);
    float screen_width();
    float screen_height();
    using resize_callback = std::function<void()>;
    int32_t resize_register(resize_callback callback);
    void resize_unregister(int32_t handle);

    // *** canvas ***
    // y goes up (same as screen position)
    void canvas_background(const NVGcolor& color);
    // canvas screen position and size is in pixels, offset, scale and size below does not influence these
    float canvas_screen_width();
    float canvas_screen_height();
    Point canvas_screen_position();
    void set_canvas_screen_size(float width, float height);
    void set_canvas_screen_position(const Point& position);
    
    // set value of lower-left point
    Point canvas_offset();
    Point canvas_scale();
    float canvas_width();
    float canvas_height();
    void set_canvas_offset(const Point& offset);
    // setting canvas size will change scale and vice-versa (screen size is not changed) 
    void set_canvas_scale(const Point& scale);
    void set_canvas_size(float width, float height);

    // *** mouse ***
    using mouse_button_callback = std::function<void()>;
    enum class mouse_button { left, right, middle };

    Point mouse_screen_position();
    Point mouse_canvas_position();
    
    bool mouse_pressed(mouse_button button);
    
    Point mouse_screen_delta();

    int32_t mouse_press_register(mouse_button button, mouse_button_callback callback);
    int32_t mouse_release_register(mouse_button button, mouse_button_callback callback);
    void mouse_unregister(int32_t handle);

    float mouse_wheel_delta();

    // *** drawing ***
    void rectangle(const Point& position, float radians, float width, float height, const NVGcolor& color);
    void circle(const Point& position, float radians, float radius, const NVGcolor& color);
    void line_directed(const Point& from, const Point& to, const NVGcolor& color);
    void line_solid(const Point& from, const Point& to, const NVGcolor& color);
    void line_dashed(const Point& from, const Point& to, const NVGcolor& color);
    void curve_quad(const Point& from, const Point& control, const Point& to, const NVGcolor& color);
    // position is number between 0 and 1 identifying point on line and offset is distance from line
    void line_text(const Point& from, const Point& to, const char* text, float position, float size, float offset, Side side, const NVGcolor& color);

    float text_width(const char* text, float size);
    float text_height(const char* text, float width, float size);
    Point text_dimensions(const char* text, float size);
    void text(const char* text, const Point& position, float width, float size, const NVGcolor& color);
    void text(const char* text, const Point& position, float size, const NVGcolor& color);

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
