#pragma once

#include <nanovg.h>
#include "sokol_gfx.h"
#include <fontstash.h>
#include "point_type.h"
#include "matrix_type_4.h"
#include "matrix_type_3.h"
#include "color_type.h"
#include <functional>
#include <vector>
#include <map>
#include <memory>

void setup();
void update();

extern NVGcontext* vg;
extern FONScontext* fons;

namespace frame
{
    static const double PI = 3.1415926535897931;
    static const float max_depth = 1000.0f;

    using vec2 = point_type<float>;
    using vec2d = point_type<double>;
    using col4 = color_type;

    using mat4 = matrix_type_4;
    using mat3 = matrix_type_3<float>;

    using vec3 = point_type_3<float>;
    using vec3d = point_type_3<double>;

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
        rectangle get_overlap(const rectangle& o) const;
        bool has_overlap(const rectangle& o) const;

        bool contains(const rectangle& o) const; // return true if rectangle o is completly inside this rect
    };

    col4 rgb(uint8_t r, uint8_t g, uint8_t b);
    col4 rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    float get_delta_time(); // time since last frame [ms]
    float get_time(); // time since start [ms]

    float deg_to_rad(float deg);
    float rad_to_deg(float rad);
    double rad_to_deg(double rad);

    // *** screen ***
    // SCREEN COORDINATES - screen coordinates has origin in upper-left and extends to right (x) and down (y)
    // screen has size get_screen_size()
    void set_screen_background(const col4& color);
    col4 get_screen_background();
    vec2 get_screen_size();
    bool is_screen_resized();

    vec2 get_screen_to_world(const vec2& screen_position);
    vec2 get_screen_to_world_vector(const vec2& screen_position);
    vec2 get_world_to_screen(const vec2& world_position);
    vec2 get_world_to_screen_vector(const vec2& world_position);

    // *** transform ***

    mat4 translation(const vec2& translation);
    mat4 rotation(float rotation);
    mat4 rotation(const vec2& center, float rotation);
    mat4 scale(const vec2& scale);
    mat4 identity();

    // 3D transform

    mat4 translation(const vec3& translation);
    mat4 scale(const vec3& scale);

    // *** world ***
    // WORLD COORDINATES - world transform is transformation matrix that transforms world to screen
    //
    // ! world transform ! - transforms world -> screen
    //
    void set_world_transform(const mat4& transform); // reset the last transform
    void set_world_transform_multiply(const mat4& transform); // reset the last transform

    void save_world_transform();
    void restore_world_transform();

    const mat4& get_world_transform();

    //vec2 get_world_translation();
    vec2 get_world_translation();
    void set_world_translation(const vec2& translation);

    //vec2 get_world_scale();
    vec2 get_world_scale();
    void set_world_scale(const vec2& scale);
    void set_world_scale(const vec2& scale, const vec2& stationary_world_point); // stationary_world_point is point that should preserve it's position

    vec2 get_world_size();   // world transformed screen size, i.e. the size of screen rectangle in world
    rectangle get_world_rectangle();

    // pos x and y is between 0 and 1, returns world position on screen at these coordinate offsets
    vec2 get_world_position_screen_relative(const vec2& rel);
    void set_world_translation(const vec2& screen_point, const vec2& world_point); // modify world translation such that screen_point match the world point

    // 3D transform

    vec3 get_world_translation_3();
    vec3 get_world_scale_3();

    void set_world_translation(const vec3& translation);
    void set_world_scale(const vec3& scale);

    // *** events ***
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

    using touch_id = uint32_t;

    const std::vector<touch_id>& get_touches_pressed();
    const std::vector<touch_id>& get_touches_released();
    const std::vector<touch_id>& get_touches_down();
    bool is_touch_down(touch_id id);
    vec2 get_touch_screen_position(touch_id id);
    vec2 get_touch_screen_delta(touch_id id);
    // touch debug
    void set_debug_touch_down(touch_id id, vec2 screen_position);
    void set_debug_touch_move(touch_id id, vec2 screen_position);
    void set_debug_touch_up(touch_id id);

    // *** drawing ***
    void draw_rectangle(const vec2& center, float width, float height, const col4& color);
    void draw_rectangle(const rectangle& rect, const col4& color);
    void draw_rectangle_ex(const vec2& center, 
                           float radians,
                           float width,
                           float height,
                           const col4& fill_color,
                           const float outline_thickness,
                           const col4& outline_color);
    void draw_rectangle_ex(const rectangle& rect,
                           float radians,
                           const col4& fill_color,
                           const float outline_thickness,
                           const col4& outline_color);
    void draw_rounded_rectangle(const vec2& center, float width, float height, float radius, const col4& color);
    void draw_rounded_rectangle_ex(const vec2& center,
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
    void draw_bezier_polyline(const std::vector<vec2>& points, const col4& color);
    void draw_bezier_polyline_ex(const std::vector<vec2>& points, float thickness, const col4& color);
    void draw_polyline(const std::vector<vec2>& points, const col4& color);
    void draw_polyline_ex(const std::vector<vec2>& points, float thickness, const col4& color);

    // flush draw calls of nanovg otherwise drawing is done at the end of update, this is hackish solution used to correctly layer
    // draws when drawing with both nanovg and sokol_gfx
    // TODO make clear distinction in api between nanovg and sokol_gfx
    void nanovg_flush();

    // *** text ***
    enum class text_align { top_left,      top_middle,      top_right, 
                            middle_left,   middle_middle,   middle_right,
                            baseline_left, baseline_middle, baseline_right,
                            bottom_left,   bottom_middle,   bottom_right };

    // TODO right now we do not support multiline string which is not aligned to the left !!!
    rectangle get_text_rectangle(const char* text, const vec2& position, float size, text_align align = text_align::top_left);
    rectangle get_text_rectangle_ex(const char* text, const vec2& position, float size, const char* font_name, text_align align = text_align::top_left);

    void draw_text(const char* text, const vec2& position, float size, const col4& color, text_align align = text_align::top_left);
    void draw_text_ex(const char* text, const vec2& position, float size, const col4& color, const char* font_name, text_align align = text_align::top_left, float blur = 0.0f, float spacing = 0.0f);
    
    struct font_metrics
    {
        float ascender;
        float descender;
        float line_height;
    };
    float draw_text_ex2(const char* text, const vec3& position, float size, const col4& color, const char* font_name, text_align align = text_align::top_left, float blur = 0.0f, float spacing = 0.0f);
    rectangle get_text_rectangle2(const char* text, const vec3& position, float size, const char* font_name, text_align align = text_align::top_left, float blur = 0.0f, float spacing = 0.0f);
    font_metrics get_font_metrics2(const char* font_name, float size);

    // TODO font won't be available right away because reading files is asynchronous, not sure whether this will be a problem
    void load_font(const char* font_name, const char* file_path);

    // *** image ***
    using image_t = uint32_t;

    image_t create_image(uint32_t width, uint32_t height, sg_pixel_format pixel_format = SG_PIXELFORMAT_RGBA8);
    image_t create_image(uint32_t width, uint32_t height, const char* data, size_t data_size, sg_pixel_format pixel_format = SG_PIXELFORMAT_RGBA8);

    struct image_target_desc
    {
        sg_pixel_format pixel_format = SG_PIXELFORMAT_RGBA8;
        uint32_t samples = 1;
        bool depth_stencil = true;

        bool operator==(const image_target_desc& b) const;
    };
    image_t create_image_target(uint32_t width, uint32_t height, const image_target_desc& desc = {});

    image_t load_image(const char* data, size_t size);
    image_t load_image(const std::vector<char>& data);

    void delete_image(image_t img);

    vec2 get_image_size(image_t img);
    sg_pixel_format get_image_pixel_format(image_t image);
    const image_target_desc& get_image_target_desc(image_t image);
    bool is_image_target(image_t image);
    //image_t get_image_target_depth_stencil(image_t target);

    void update_image(image_t image, const char* data, size_t data_size);

    struct image_draw_desc
    {
        sg_filter filter = SG_FILTER_LINEAR;
        sg_wrap wrap = SG_WRAP_CLAMP_TO_BORDER;
        col4 tint_color = col4::WHITE;

        bool operator==(const image_draw_desc& draw_desc) const;
    };

    void draw_image(image_t img, const vec2& position, text_align align = text_align::top_left, image_draw_desc draw_desc = {});
    void draw_image_ex(image_t img, const vec2& position, float radians, const vec2& scale, text_align align = text_align::top_left, image_draw_desc draw_desc = {});
    void draw_image_ex_size(image_t img, const vec2& position, float radians, const vec2& screen_size, text_align align = text_align::top_left, image_draw_desc draw_desc = {});

    // *** pass ***
    void begin_pass(image_t target);
    void begin_pass_clear(image_t target, const col4& color);
    void begin_default_pass();
    void end_pass();

    // *** file ***
    using fetch_callback = std::function<void(std::vector<char>)>;

    void fetch_file(const char* file_path, fetch_callback callback, size_t file_size_hint = 0);
    void fetch_files(const std::vector<std::string>& files, std::function<void(std::map<std::string, std::vector<char>> file_data)> finished_callback);
}
