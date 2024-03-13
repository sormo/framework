#pragma once

#include <nanovg.h>
#include "point_type.h"
#include "matrix_type.h"
#include "color_type.h"
#include <functional>

void setup();
void update();

extern NVGcontext* vg;

namespace frame
{
    using vec2 = point_type<float>;
    using vec2d = point_type<double>;
    using col4 = color_type;
    using mat3 = matrix_type<float>;
    using mat3d = matrix_type<double>;

    // TODO possibly move to separate file
    struct rectangle
    {
        static rectangle from_min_max(const vec2& min, const vec2& max);
        static rectangle from_center_size(const vec2& center, const vec2& size);

        vec2 min;
        vec2 max;

        vec2 center() const;
        vec2 size() const;

        bool contains(const vec2& o) const;
        rectangle overlap(const rectangle& o) const;
    };

    col4 rgb(char r, char g, char b);
    col4 rgba(char r, char g, char b, char a);

    float get_delta_time(); // time since last frame [ms]
    float get_time(); // time since start [ms]

    float deg_to_rad(float deg);
    float rad_to_deg(float rad);

    // *** screen ***
    void set_screen_background(const col4& color);
    const col4& get_screen_background();
    vec2 get_screen_size();
    bool is_screen_resized();

    // *** transform ***
    mat3 translation(const vec2& translation);
    mat3 rotation(float rotation);
    mat3 scale(const vec2& scale);
    mat3 identity();

    // *** world ***
    void set_world_transform(const mat3& transform); // reset the last transform
    void set_world_transform_multiply(const mat3& transform); // reset the last transform

    void save_world_transform();
    void restore_world_transform();

    const mat3& get_world_transform();

    vec2 get_world_translation();
    void set_world_translation(const vec2& translation);

    vec2 get_world_scale();
    void set_world_scale(const vec2& scale);
    void set_world_scale(const vec2& scale, const vec2& stationary_world_point); // stationary_world_point is point that should preserve it's position

    vec2 get_world_size();   // world transformed screen size, i.e. the size of screen rectangle in world
    rectangle get_world_rectangle();

    // pos x and y is between 0 and 1, returns world position on screen at these coordinate offsets
    vec2 get_world_position_screen_relative(const vec2& rel);

    // *** mouse ***
    enum class mouse_button { left, right, middle };
    vec2 get_mouse_screen_position();
    vec2 get_mouse_world_position();

    bool is_mouse_down(mouse_button button);
    bool is_mouse_pressed(mouse_button button);
    bool is_mouse_released(mouse_button button);
    bool is_mouse_scrolled();

    vec2 get_mouse_screen_delta();
    float get_mouse_wheel_delta();

    bool is_key_down(char key);
    bool is_key_pressed(char key);
    bool is_key_released(char key);

    // *** drawing ***
    void draw_rectangle(const vec2& position, float width, float height, const col4& color);
    void draw_rectangle_ex(const vec2& position, 
                           float radians,
                           float width,
                           float height,
                           const col4& fill_color,
                           const float outline_thickness,
                           const col4& outline_color);
    void draw_rounded_rectangle(const vec2& position, float width, float height, float radius, const col4& color);
    void draw_rounded_rectangle_ex(const vec2& position, 
                                   float radians,
                                   float width,
                                   float height,
                                   float radius,
                                   const col4& fill_color,
                                   const float outline_thickness,
                                   const col4& outline_color);
    void draw_circle(const vec2& position, float radius, const col4& color);
    void draw_circle_ex(const vec2& position,
                        float radians,
                        float radius,
                        const col4& fill_color,
                        const float outline_thickness,
                        const col4& outline_color);
    void draw_ellipse(const vec2& position,float major, float minor, const col4& color);
    void draw_ellipse_ex(const vec2& position,
                         float radians,
                         float major,
                         float minor,
                         const col4& fill_color,
                         const float outline_thickness,
                         const col4& outline_color);
    void draw_hyperbola(const vec2& position,
                        float radians,
                        float major,
                        float minor,
                        float outline_thickness,
                        const col4& outline_color);
    void draw_polygon(const vec2& position, const vec2* vertices, size_t count, const col4& color);
    void draw_polygon_ex(const vec2& position,
                         float radians,
                         const vec2* vertices,
                         size_t count,
                         const col4& fill_color,
                         const float outline_thickness,
                         const col4& outline_color);
    void draw_line_directed(const vec2& from, const vec2& to, const col4& color);
    void draw_line_directed_ex(const vec2& from, const vec2& to, float thickness, const col4& color);
    void draw_line_solid(const vec2& from, const vec2& to, const col4& color);
    void draw_line_solid_ex(const vec2& from, const vec2& to, float thickness, const col4& color);
    void draw_line_dashed(const vec2& from, const vec2& to, const col4& color);
    void draw_line_dashed_ex(const vec2& from, const vec2& to, float thickness, const col4& color);
    void draw_quad_bezier(const vec2& from, const vec2& control, const vec2& to, const col4& color);
    void draw_quad_bezier_ex(const vec2& from, const vec2& control, const vec2& to, float thickness, const col4& color);
    void draw_quad_bezier_polyline(const std::vector<vec2>& points, const col4& color);
    void draw_quad_bezier_polyline_ex(const std::vector<vec2>& points, float thickness, const col4& color);
    void draw_polyline(const std::vector<vec2>& points, const col4& color);
    void draw_polyline_ex(const std::vector<vec2>& points, float thickness, const col4& color);

    // *** text ***
    enum class text_align { top_left,    top_middle,    top_right, 
                            middle_left, middle_middle, middle_right,
                            bottom_left, bottom_midle,  bottom_right };

    rectangle get_text_rectangle(const char* text, const vec2& position, float size, text_align align = text_align::top_left);

    void draw_text(const char* text, const vec2& position, float size, const col4& color, text_align align = text_align::top_left);
}
