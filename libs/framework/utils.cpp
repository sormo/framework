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
        return std::uniform_real_distribution<double>(min, max)(g_random_generator);
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

        return translation + get_world_to_screen_vector(vec);
    }

    void free_move_camera_update_world_offset(mouse_button button, const rectangle& boundary, bool allow_touch)
    {
        if (is_mouse_down(button) || (allow_touch && frame::get_touches_down().size() == 1))
        {
            auto translation_delta = is_mouse_down(button) ? get_mouse_screen_delta() : get_touch_screen_delta(get_touches_down()[0]);

            vec2 new_translation = get_world_translation() + translation_delta;
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

    void free_move_camera_update_world_scale(float zoom_speed, const vec2& min_size, const vec2& max_size, bool allow_touch)
    {
        if (get_mouse_wheel_delta())
        {
            vec2 new_scale = get_world_scale() * (1.0f + get_mouse_wheel_delta() * zoom_speed);
            new_scale = free_move_camera_apply_min_size(new_scale, min_size);
            new_scale = free_move_camera_apply_max_size(new_scale, max_size);
            set_world_scale(new_scale, get_mouse_world_position());
        }

        if (allow_touch && get_touches_down().size() >= 2)
        {
            auto delta1 = get_touch_screen_delta(get_touches_down()[0]);
            auto delta2 = get_touch_screen_delta(get_touches_down()[1]);
            auto pos1 = get_touch_screen_position(get_touches_down()[0]);
            auto pos2 = get_touch_screen_position(get_touches_down()[1]);
            auto mid = pos1 + (pos2 - pos1) / 2.0f;
            auto prev_pos1 = pos1 - delta1;
            auto prev_pos2 = pos2 - delta2;
            auto prev_mid = prev_pos1 + (prev_pos2 - prev_pos1) / 2.0f;

            auto delta_pos = mid - prev_mid;
            auto delta_scale = (pos2 - pos1).length() - (prev_pos2 - prev_pos1).length();

            set_world_translation(get_world_translation() + delta_pos);
            vec2 new_scale = get_world_scale() * (1.0f + delta_scale * zoom_speed);
            set_world_scale(new_scale, get_screen_to_world(mid));
        }
    }

    void free_move_camera_update(const free_move_camera_config& config)
    {
        if (config.allow_scale)
            free_move_camera_update_world_scale(config.zoom_speed, config.min_size, config.boundary.size(), config.allow_touch);
        if (config.allow_move)
            free_move_camera_update_world_offset(config.move_button, config.boundary, config.allow_touch);
    }

    void draw_coordinate_lines(const col4& color)
    {
        auto world_rect = get_world_rectangle();
        float thickness = 1.0f / get_world_scale().abs().x;
        auto world_size = get_world_size();

        auto draw_lines_x = [&world_rect](double step, const col4& color, double thickness)
        {
            double start_x = std::ceil((double)world_rect.min.x / step) * step;
            for (double x = start_x; x < (double)world_rect.max.x; x += step)
                draw_line_solid_ex({ (float)x, world_rect.min.y }, { (float)x, world_rect.max.y }, x == 0.0 ? (float)(2.0 * thickness) : (float)thickness, color);
        };

        auto draw_lines_y = [&world_rect](double step, const col4& color, double thickness)
        {
            double start_y = std::ceil((double)world_rect.min.y / step) * step;
            for (double y = start_y; y < (double)world_rect.max.y; y += step)
                draw_line_solid_ex({ world_rect.min.x, (float)y }, { world_rect.max.x, (float)y }, y == 0.0 ? (float)(2.0 * thickness) : (float)thickness, color);
        };

        auto draw_grid = [&color, thickness](double world_size, auto draw_lines)
        {
            // using double here, to support large scale
            // if we use float and step is small (like e-5) it won't increment the y coordinate in the for loop leading to infinte loop
            double major_power = std::floor(log10(world_size));
            double major_step = std::pow(10.0, major_power);
            double minor_step = std::pow(10.0, major_power - 1.0);
            double major_size = major_step / world_size;

            draw_lines(major_step, color, thickness);
            draw_lines(minor_step, color.transparency(uint8_t(255 * color.alpha() * (float)major_size)), thickness * major_size);
        };

        draw_grid(world_size.x, draw_lines_x);
        draw_grid(world_size.y, draw_lines_y);
    }

    bool is_inside_line_handle(const vec2& position, const vec2& handle_position, line_handle_config* handle)
    {
        if (handle == nullptr)
            return false;
        return (position - handle_position).length_sqr() < handle->radius * handle->radius;
    }

    void draw_line_handle(const vec2& position, line_handle_config& handle)
    {
        float world_scale = get_world_scale().x;

        if ((position - get_mouse_world_position()).length_sqr() < handle.radius * handle.radius)
            draw_circle_ex(position, 0.0f, handle.radius / world_scale, handle.hover_fill_color, handle.hover_outline_thickness / world_scale, handle.hover_outline_color);
        else
            draw_circle_ex(position, 0.0f, handle.radius / world_scale, handle.fill_color, handle.outline_thickness / world_scale, handle.outline_color);
    }

    void draw_line_directed_with_handles(const vec2& from, const vec2& to, float thickness, const col4& color, line_handle_config* from_handle, line_handle_config* to_handle)
    {
        float world_scale = get_world_scale().x;

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

        auto world_dt = get_screen_to_world_vector(get_mouse_screen_delta());
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

    click_handler::click_handler(frame::mouse_button button, bool allow_touch)
        : button(button), allow_touch(allow_touch)
    {
    }

    vec2 click_handler::get_pressed_screen_position()
    {
        if (frame::is_mouse_pressed(button))
            return frame::get_mouse_screen_position();
        if (allow_touch && frame::get_touches_pressed().size() == 1)
            return frame::get_touch_screen_position(frame::get_touches_pressed()[0]);
        return {};
    }

    vec2 click_handler::get_released_screen_position()
    {
        if (frame::is_mouse_released(button))
            return frame::get_mouse_screen_position();

        auto release_touch = std::find(get_touches_released().begin(), get_touches_released().end(), click_touch);
        if (allow_touch && release_touch != get_touches_released().end())
            return frame::get_touch_screen_position(*release_touch);
        return {};
    }

    void click_handler::update()
    {
        clicked = false;

        if (frame::is_mouse_pressed(button) || (allow_touch && frame::get_touches_pressed().size() == 1))
        {
            press_ticks = stm_now();
            press_position_screen = get_pressed_screen_position();

            if (frame::get_touches_pressed().size() == 1)
                click_touch = frame::get_touches_pressed()[0];
        }
        else if (frame::is_mouse_released(button) || (allow_touch && std::find(get_touches_released().begin(), get_touches_released().end(), click_touch) != frame::get_touches_released().end()))
        {
            auto duration_ms = stm_ms(stm_since(press_ticks));
            auto distance_px = (get_released_screen_position() - press_position_screen).length();

            clicked = duration_ms < max_click_duration && distance_px < max_click_distance;

            click_touch = (touch_id)-1;
        }
    }

    bool click_handler::is_clicked()
    {
        return clicked;
    }

    frame::vec2 click_handler::get_click_screen_position()
    {
        return press_position_screen;
    }

    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


    static inline bool is_base64(char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string base64_encode(const char* buffer, size_t size)
    {
        std::string ret;
        int i = 0;
        int j = 0;
        char char_array_3[3];
        char char_array_4[4];

        while (size--)
        {
            char_array_3[i++] = *(buffer++);
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += base64_chars[char_array_4[j]];

            while ((i++ < 3))
                ret += '=';
        }

        return ret;
    }

    std::vector<char> base64_decode(const char* string)
    {
        int in_len = (int)strlen(string);
        int i = 0;
        int j = 0;
        int in_ = 0;
        char char_array_4[4], char_array_3[3];
        std::vector<char> ret;

        while (in_len-- && (string[in_] != '=') && is_base64(string[in_]))
        {
            char_array_4[i++] = string[in_]; in_++;
            if (i == 4)
            {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = (char)base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = (char)base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++)
                ret.push_back(char_array_3[j]);
        }

        return ret;
    }
}
