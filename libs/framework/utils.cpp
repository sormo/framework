#include "utils.h"
#include <random>

std::mt19937_64 g_random_generator;

namespace frame
{
    namespace detail
    {
        // return number between 0 to 1 of current eighth
        float get_angle_eighth(float radians)
        {
            return std::fmodf(radians, NVG_PI / 4.0f) / (NVG_PI / 4.0f);
        }

        direction get_start(float radians)
        {
            if (radians < 1.0f * NVG_PI / 4.0f)
                return direction::right;
            else if (radians < 2.0f * NVG_PI / 4.0f)
                return direction::up_right;
            else if (radians < 3.0f * NVG_PI / 4.0f)
                return direction::up;
            else if (radians < 4.0f * NVG_PI / 4.0f)
                return direction::up_left;
            else if (radians < 5.0f * NVG_PI / 4.0f)
                return direction::left;
            else if (radians < 6.0f * NVG_PI / 4.0f)
                return direction::down_left;
            else if (radians < 7.0f * NVG_PI / 4.0f)
                return direction::down;
            else if (radians < 8.0f * NVG_PI / 4.0f)
                return direction::down_right;
            return direction::right; // never happen
        }

        direction get_end(float radians)
        {
            radians += NVG_PI / 4.0f;
            if (radians > 2.0f * NVG_PI)
                radians -= 2.0f * NVG_PI;

            return get_start(radians);
        }

        // return point on text rectangle which should match with line on point
        vec2 get_rect_point(direction direction)
        {
            switch (direction)
            {
            case direction::right:
                return { -0.5f,  0.0f };
            case direction::up_right:
                return { -1.0f,  0.0f };
            case direction::up:
                return { -1.0f, -0.5f };
            case direction::up_left:
                return { -1.0f, -1.0f };
            case direction::left:
                return { -0.5f, -1.0f };
            case direction::down_left:
                return { 0.0f, -1.0f };
            case direction::down:
                return { 0.0f, -0.5f };
            case direction::down_right:
                return { 0.0f,  0.0f };
            }
            return {}; // never happen
        }
    }

    float randf()
    {
        return std::uniform_real_distribution<float>(0.0f, 1.0f)(g_random_generator);
    }

    double rand()
    {
        return std::uniform_real_distribution<double>(0.0, 1.0)(g_random_generator);
    }

    float randf(float min, float max)
    {
        return std::uniform_real_distribution<float>(min, max)(g_random_generator);
    }

    double rand(double min, double max)
    {
        return std::uniform_real_distribution<float>(min, max)(g_random_generator);
    }

    vec2 get_direction_vector(direction direction)
    {
        switch (direction)
        {
            case direction::right:
                return { 1.0f, 0.0f };
            case direction::up_right:
                return { 0.5f, 0.5f };
            case direction::up:
                return { 0.0f, 1.0f };
            case direction::up_left:
                return { -0.5f, 0.5f };
            case direction::left:
                return { -1.0f, 0.0f };
            case direction::down_left:
                return { -0.5f, -0.5f };
            case direction::down:
                return { 0.0f, -1.0f };
            case direction::down_right:
                return { 0.5f, -0.5f };
        }
        return {}; // never happen
    }

    void draw_line_text(const vec2& from, const vec2& to, const char* text, float position, float size, float offset, side side, const col4& color)
    {
        vec2 lineVector = to - from;
        float lineAngle = lineVector.angle();
        vec2 linevec2 = from + lineVector * position;
        auto textRect = get_text_rectangle(text, {}, size);

        // apply offset
        linevec2 = linevec2 + lineVector.perpendicular(side).normalized() * offset;

        // TODO this most probably can be simplified with align
        vec2 textStart = linevec2, textEnd = linevec2;
        if (side == side::left)
        {
            textStart = textStart + textRect.size() * detail::get_rect_point(detail::get_start(lineAngle));
            textStart = textStart + textRect.size() / 2.0f;
            textEnd = textEnd + textRect.size() * detail::get_rect_point(detail::get_end(lineAngle));
            textEnd = textEnd + textRect.size() / 2.0f;
        }
        else
        {
            textStart = textStart - textRect.size() * detail::get_rect_point(detail::get_start(lineAngle));
            textStart = textStart - textRect.size() / 2.0f;
            textEnd = textEnd - textRect.size() * detail::get_rect_point(detail::get_end(lineAngle));
            textEnd = textEnd - textRect.size() / 2.0f;
        }

        vec2 rectPosition = textStart.interpolate(textEnd, detail::get_angle_eighth(lineAngle));
        //rectangle(rectPosition, 0.0f, textRect.size().x, textRect.size().y, nvgRGBA(50, 80, 60, 120));
        draw_text(text, rectPosition - textRect.size() / 2.0f, size, color);
    }

    vec2 free_move_camera_apply_boundary(const vec2& translation, const rectangle& boundary)
    {
        vec2 vec = {};
        auto world_rect = get_world_rectangle();

        if (world_rect.min.x < boundary.min.x)
            vec.x = world_rect.min.x - boundary.min.x;
        else if (world_rect.max.x > boundary.max.x)
            vec.x = world_rect.max.x - boundary.max.x;

        if (world_rect.min.y < boundary.min.y)
            vec.y = world_rect.min.y - boundary.min.y;
        else if (world_rect.max.y > boundary.max.y)
            vec.y = world_rect.max.y - boundary.max.y;

        return translation + get_world_transform().transform_vector(vec);
    }

    void free_move_camera_update_world_offset(mouse_button button, const rectangle& boundary)
    {
        if (is_mouse_down(button))
        {
            vec2 new_translation = get_world_translation() + get_mouse_screen_delta();
            set_world_translation(new_translation);
            new_translation = free_move_camera_apply_boundary(new_translation, boundary);
            set_world_translation(new_translation);
        }
    }

    vec2 free_move_camera_apply_min_size(const vec2& scale, const vec2& min_size)
    {
        // scale is used to transform world to screen so we need to divide to get the other way
        auto size = get_screen_size() / scale.abs();
        auto diff = size - min_size;

        if (diff.x < 0.0f || diff.y < 0.0f)
        {
            auto scale_to_min = get_screen_size() / min_size;
            auto sign = scale.sign();

            if (scale_to_min.x < scale_to_min.y)
                return sign * vec2{ scale_to_min.x, std::abs(scale.y / scale.x) * scale_to_min.x };
            else
                return sign * vec2{ std::abs(scale.x / scale.y) * scale_to_min.y, scale_to_min.y };
        }

        return scale;
    }

    vec2 free_move_camera_apply_max_size(const vec2& scale, const vec2& max_size)
    {
        // scale is used to transform world to screen so we need to divide to get the other way
        auto size = get_screen_size() / scale.abs();
        auto diff = max_size - size;

        if (diff.x < 0.0f || diff.y < 0.0f)
        {
            auto scale_to_max = get_screen_size() / max_size;
            auto sign = scale.sign();

            if (scale_to_max.x > scale_to_max.y)
                return sign * vec2{ scale_to_max.x, std::abs(scale.y / scale.x) * scale_to_max.x };
            else
                return sign * vec2{ std::abs(scale.x / scale.y) * scale_to_max.y, scale_to_max.y };
        }

        return scale;
    }

    void free_move_camera_update_world_scale(float zoom_speed, const vec2& min_size, const vec2& max_size)
    {
        if (get_mouse_wheel_delta())
        {
            vec2 new_scale = get_world_transform().get_scale() * (1.0f + get_mouse_wheel_delta() * zoom_speed);
            new_scale = free_move_camera_apply_min_size(new_scale, min_size);
            new_scale = free_move_camera_apply_max_size(new_scale, max_size);
            set_world_scale(new_scale, get_mouse_world_position());
        }
    }

    void free_move_camera_update(const free_move_camera_config& config)
    {
        if (config.allow_scale)
            free_move_camera_update_world_scale(config.zoom_speed, config.min_size, config.boundary.size());
        if (config.allow_move)
            free_move_camera_update_world_offset(config.move_button, config.boundary);
    }

    void draw_coordinate_lines(const col4& color)
    {
        save_world_transform();

        auto world_rect = get_world_rectangle();
        float thickness = 1.0f / get_world_transform().get_scale().abs().x;
        auto world_size = get_world_size();

        auto draw_lines_x = [&world_rect](float step, const col4& color, float thickness)
        {
            float start_x = std::ceil(world_rect.min.x / step) * step;
            for (float x = start_x; x < world_rect.max.x; x += step)
                draw_line_solid_ex({ x, world_rect.min.y }, { x, world_rect.max.y }, x == 0.0f ? 2.0f * thickness : thickness, color);
        };

        auto draw_lines_y = [&world_rect](float step, const col4& color, float thickness)
        {
            float start_y = std::ceil(world_rect.min.y / step) * step;
            for (float y = start_y; y < world_rect.max.y; y += step)
                draw_line_solid_ex({ world_rect.min.x, y }, { world_rect.max.x, y }, y == 0.0f ? 2.0f * thickness : thickness, color);
        };

        auto draw_grid = [&color, thickness](float world_size, auto draw_lines)
        {
            float major_power = std::floor(log10(world_size));
            float major = std::pow(10.0f, major_power);
            float minor = std::pow(10.0f, major_power - 1.0f);
            float major_size = major / world_size;

            draw_lines(major, color, thickness);
            draw_lines(minor, color.transparency(255 * color.alpha() * major_size), thickness * major_size);
        };

        draw_grid(world_size.x, draw_lines_x);
        draw_grid(world_size.y, draw_lines_y);

        restore_world_transform();
    }

    bool is_inside_line_handle(const vec2& position, const vec2& handle_position, line_handle_config* handle)
    {
        if (handle == nullptr)
            return false;
        return (position - handle_position).length_sqr() < handle->radius * handle->radius;
    }

    void draw_line_handle(const vec2& position, line_handle_config& handle)
    {
        float world_scale = get_world_transform().get_scale().x;

        if ((position - get_mouse_world_position()).length_sqr() < handle.radius * handle.radius)
            draw_circle_ex(position, 0.0f, handle.radius / world_scale, handle.hover_fill_color, handle.hover_outline_thickness / world_scale, handle.hover_outline_color);
        else
            draw_circle_ex(position, 0.0f, handle.radius / world_scale, handle.fill_color, handle.outline_thickness / world_scale, handle.outline_color);
    }

    void draw_line_directed_with_handles(const vec2& from, const vec2& to, float thickness, const col4& color, line_handle_config* from_handle, line_handle_config* to_handle)
    {
        float world_scale = get_world_transform().get_scale().x;

        draw_line_directed_ex(from, to, thickness / world_scale, color);
        if (from_handle)
            draw_line_handle(from, *from_handle);
        if (to_handle)
            draw_line_handle(to, *to_handle);
    }

    bool update_line_with_handles(vec2& from, vec2& to, line_handle_config* from_handle, line_handle_config* to_handle)
    {
        if (!is_mouse_down(mouse_button::left))
            return false;

        auto world_dt = get_world_transform().inverted().transform_vector(get_mouse_screen_delta());
        auto position = get_mouse_world_position();

        if (is_inside_line_handle(position - world_dt, from, from_handle))
        {
            from += world_dt;
            return true;
        }
        if (is_inside_line_handle(position - world_dt, to, to_handle))
        {
            to += world_dt;
            return true;
        }
        return false;
    }

    std::string make_lower_case_string(const std::string& str)
    {
        std::string result(str);
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);

        return result;
    }

    click_handler::click_handler(frame::mouse_button button)
        : button(button)
    {
    }

    void click_handler::update()
    {
        clicked = false;

        if (frame::is_mouse_pressed(button))
        {
            press_ticks = stm_now();
            press_position_screen = frame::get_mouse_screen_position();
        }
        else if (frame::is_mouse_released(button))
        {
            auto duration_ms = stm_ms(stm_since(press_ticks));
            auto distance_px = (frame::get_mouse_screen_position() - press_position_screen).length();

            clicked = duration_ms < max_click_duration && distance_px < max_click_distance;
        }
    }

    bool click_handler::is_clicked()
    {
        return clicked;
    }
}
