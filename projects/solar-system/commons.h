#pragma once
#include "unit.h"

namespace commons
{
    static const double MIN_ZOOMED_SIZE = 0.1;
    static const double DRAW_SIZE_FACTOR = 200.0;
    static const double DRAW_VELOCITY_FACTOR = 13.0;

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
        bodies_included_type bodies_included = bodies_included_type::more_than_100;

        bool body_system_initializing = false;
    };

    static frame::vec2 draw_cast(const frame::vec3d& p)
    {
        return { (float)(p.x * DRAW_SIZE_FACTOR), (float)(p.y * DRAW_SIZE_FACTOR) };
    }

    static float pixel_to_world(float s)
    {
        return s / frame::get_world_scale().x;
    }

    static float world_to_pixel(float s)
    {
        return s * frame::get_world_scale().x;
    }

    static std::vector<frame::vec2> draw_cast(const std::vector<frame::vec3d>& data)
    {
        std::vector<frame::vec2> r;
        r.reserve(data.size());
        for (const auto& o : data)
            r.push_back(draw_cast(o));
        return r;
    }

    static double convert_world_size_to_AU(double world_size)
    {
        return unit::AU * world_size / commons::DRAW_SIZE_FACTOR;
    }

    static double convert_km_to_world_size(double value_km)
    {
        return value_km * commons::DRAW_SIZE_FACTOR * unit::kilometer;
    }

    static double convert_AU_to_world_size(double value_au)
    {
        return value_au * commons::DRAW_SIZE_FACTOR * unit::AU;
    }

    static double convert_AU_to_km(double value_au)
    {
        return value_au * 1.496e+8;
    }

    static std::string convert_double_to_string(double num)
    {
        std::string text(256, '\0');
        if (num < 0.000'001 || num > 100'000.0)
            std::sprintf(text.data(), "%1.2e", num);
        else
            std::sprintf(text.data(), "%.3f", num);
        text.resize(strlen(text.data()));
        return text;
    }

    // https://easings.net/#easeOutCubic
    static float easing_cubic_out(float x)
    {
        return 1.0f - std::pow(1.0f - x, 3.0f);
    }
}
