#pragma once

#include "framework.h"
#include <sokol_time.h>
#include <optional>

namespace frame
{
    // https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
    template <typename T> 
    int sign(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    enum class direction
    {
        right,
        up_right,
        up,
        up_left,
        left,
        down_left,
        down,
        down_right
    };
    vec2 get_direction_vector(direction direction);

    // random value between 0 to 1
    float randf();
    double rand();

    float randf(float min, float max);
    double rand(double min, double max);

    //https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
    std::string base64_encode(const char* buffer, size_t size);
    std::vector<char> base64_decode(const char* string);

    //enum class side
    //{
    //    left,
    //    right
    //};

    // Draw a text alongside of a line.
    // position is number between 0 and 1 identifying point on line and offset is distance from line.
    void draw_line_text(const vec2& from, const vec2& to, const char* text, float position, float size, float offset, side side, const col4& color);

    // Provides functionality for moving camera with mouse and zooming with mouse wheel.
    // Call this function in update method.
    struct free_move_camera_config
    {
        mouse_button move_button = mouse_button::right;
        bool allow_move = true;
        bool allow_scale = true;
        bool allow_touch = true;
        vec2 min_size = {}; // minimum size of world rectangle
        rectangle boundary = { {}, { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() }};
        float zoom_speed = 0.01f;
    };
    void free_move_camera_update(const free_move_camera_config& config);

    void draw_coordinate_lines(const col4& color);

    struct line_handle_config
    {
        float radius;

        col4 fill_color = col4::BLANK;
        col4 outline_color;
        float outline_thickness = 1.0f;

        col4 hover_fill_color = col4::BLANK;
        col4 hover_outline_color;
        float hover_outline_thickness = 1.0f;
    };
    void draw_line_directed_with_handles(const vec2& from, const vec2& to, float thickness, const col4& color, line_handle_config* from_handle, line_handle_config* to_handle);
    bool update_line_with_handles(vec2& from, vec2& to, line_handle_config* from_handle, line_handle_config* to_handle);

    std::string make_lower_case_string(const std::string& str);

    // std::vector<vec2> create_bezier_curve(const std::vector<vec2>& polyline); TODO

    struct click_handler
    {
        static constexpr float max_click_duration = 500.0f; // milliseconds
        static constexpr float max_click_distance = 5.0f; // pixels

        click_handler(frame::mouse_button button, bool allow_touch = true);

        void update();
        bool is_clicked();

        frame::vec2 get_click_screen_position();

    private:
        vec2 get_pressed_screen_position();
        vec2 get_released_screen_position();

        const frame::mouse_button button;
        const bool allow_touch;

        uint64_t press_ticks = 0;
        vec2 press_position_screen;
        bool clicked = false;

        touch_id click_touch = (touch_id)-1;
    };
}
