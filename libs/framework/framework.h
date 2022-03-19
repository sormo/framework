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

    // TODO
    Point convert_from_gui_to_screen(const Point& position);
    Point convert_from_canvas_to_screen(const Point& position);
    Point convert_from_screen_to_gui(const Point& position);
    Point convert_from_screen_to_canvas(const Point& position);

    // *** screen ***
    void screen_background(const NVGcolor& color);
    float screen_width();
    float screen_height();
    // helper function to notify about resize
    using resize_callback = std::function<void()>;
    int32_t resize_register(resize_callback callback);
    void resize_unregister(int32_t handle);

    // *** canvas ***
    void canvas_background(const NVGcolor& color);
    float canvas_width();
    float canvas_height();
    void set_canvas_size(float width, float height);
    // lower-left corner of canvas (screen coordinates)
    Point canvas_position();
    // set lower-left position of canvas
    void set_canvas_position(const Point& position);

    // *** mouse ***
    // screen position is relative to lower-left
    Point mouse_screen_position();
    bool mouse_pressed();
    // helper function to track mouse press event
    using mouse_callback = std::function<void()>;
    int32_t mouse_press_register(mouse_callback callback);
    void mouse_press_unregister(int32_t handle);
    int32_t mouse_release_register(mouse_callback callback);
    void mouse_release_unregister(int32_t handle);

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
