#pragma once
#include "unit.h"
#include "drawing_sg.h"
#include <utils.h>

struct body_node;

namespace commons
{
    static const double MIN_ZOOMED_SIZE = 0.1;
    static const double DRAW_SIZE_FACTOR = 200.0;
    static const double DRAW_DEPTH_FACTOR = 10.0;
    static const double DRAW_VELOCITY_FACTOR = 13.0;

    // font size for body names (when not zoomed in)
    static const float NAME_FONT_SIZE = 15.0f;

    static const time_t INIT_TIME_EPOCH = 1711843200; // 2024.03.31 00:00:00
    static const double INIT_TIME_JULIAN = 2460034.5; // 2024.03.31 00:00:00

    enum bodies_included_type : int
    {
        more_than_100,
        more_than_50,
        more_than_10
    };

    struct settings_data
    {
        bool draw_trajectories = true;
        bool draw_names = true;
        bool draw_points = true;
        bool step_time = true;
        float step_speed = 0.001f; // in days
        bool disable_inclination = false;
        bool draw_lagrangians = false;
        bool shaded_planets = true;
        bool model_rotation_random = false;
        bool show_rotation_axis = true;
        bodies_included_type bodies_included = bodies_included_type::more_than_100;
    };
    
    struct state_data
    {
        bool body_system_initializing = false;
        body_node* clicked_body = nullptr;
        body_node* view_body = nullptr;
        double time_offset = 0.0; // in days 
    };

    [[maybe_unused]] static frame::vec3 draw_cast(const frame::vec3d& p)
    {
        return { (float)(p.x * DRAW_SIZE_FACTOR), (float)(p.y * DRAW_SIZE_FACTOR), (float)frame::clamp(p.z * DRAW_DEPTH_FACTOR, -frame::max_depth, frame::max_depth) };
    }

    [[maybe_unused]] static float pixel_to_world(float s)
    {
        return s / frame::get_world_scale().x;
    }

    [[maybe_unused]] static float world_to_pixel(float s)
    {
        return s * frame::get_world_scale().x;
    }

    [[maybe_unused]] static std::vector<frame::vec3> draw_cast(const std::vector<frame::vec3d>& data, double scale = 1.0)
    {
        std::vector<frame::vec3> r;
        r.reserve(data.size());
        for (const auto& o : data)
            r.push_back(draw_cast(frame::vec3d{ o.x * scale, o.y * scale, o.z }));
        return r;
    }

    [[maybe_unused]] static double convert_world_size_to_AU(double world_size)
    {
        return unit::AU * world_size / commons::DRAW_SIZE_FACTOR;
    }

    [[maybe_unused]] static double convert_km_to_world_size(double value_km)
    {
        return value_km * commons::DRAW_SIZE_FACTOR * unit::kilometer;
    }

    [[maybe_unused]] static double convert_AU_to_world_size(double value_au)
    {
        return value_au * commons::DRAW_SIZE_FACTOR * unit::AU;
    }

    [[maybe_unused]] static double convert_AU_to_km(double value_au)
    {
        return value_au * 1.496e+8;
    }

    [[maybe_unused]] static std::string convert_double_to_string(double num)
    {
        std::string text(256, '\0');
        if (std::fabs(num) < 0.000'001 || std::fabs(num) > 100'000.0)
            std::sprintf(text.data(), "%1.2e", num);
        else
            std::sprintf(text.data(), "%.3f", num);
        text.resize(strlen(text.data()));
        return text;
    }

    // https://easings.net/#easeOutCubic
    [[maybe_unused]] static float easing_cubic_out(float x)
    {
        return 1.0f - std::pow(1.0f - x, 3.0f);
    }
}

extern commons::settings_data settings;
extern commons::state_data state;