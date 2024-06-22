#pragma once

#include "framework.h"
#include "view.h"
#include "drawing_sg.h"
#include <kepler_orbit.h>

struct trajectories_draw_cache
{
    void draw(frame::draw_buffer_id id, frame::mat3&& transform, frame::col4&& color);

    void flush();

    std::vector<frame::draw_buffer_id> ids;
    std::vector<frame::mat3> transforms;
    std::vector<frame::col4> colors;
};

struct trajectory_resolutions
{
    static const int max_orbit_points = 4000;

    static trajectories_draw_cache draw_cache;

    struct resmap_type
    {
        int point_count;
        float semi_major_axis_pixel_size;
    };

    std::array<resmap_type, 7> resmap =
    {{
        { 40, 600 },
        { 120, 2000 },
        { 400, 4000 },
        { 800, 8000 },
        { 1600, 16'000 },
        { 3200, 32'000 },
        { max_orbit_points, 64'000 }
    }};

    void draw(const frame::vec2& world_translation, double semi_major_axis_world_size, bool has_stationary_parent);

    void init(kepler_orbit& orbit);

    std::vector<frame::vec2>& get_points();

private:
   
    struct resolution
    {
        double radius = 0.0;
        int point_count = 0;
        frame::draw_buffer_id draw_id = frame::draw_buffer_id_invalid;
    };

    resolution& get_resolution(double semi_major_axis_pixel_size);
    frame::mat3 get_transform(const frame::vec2& position, bool has_stationary_parent);

    frame::draw_buffer_id create_trajectory(const std::vector<frame::vec2>& points, int point_count);

    double scale_factor = 1.0;

    std::vector<resolution> resolutions;
    std::vector<frame::vec2> points;

    std::optional<frame::mat3> cached_transform;
};
