#pragma once
#include "commons.h"
#include "framework.h"
#include "view.h"
#include "drawing_sg.h"
#include <kepler_orbit.h>

struct trajectory_resolutions
{
    static const int max_orbit_points = 4000;

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

    void draw(double semi_major_axis_world_size)
    {
        auto get_color = [](int point_count)
        {
#ifdef _DEBUG
            if (point_count == 40)
                return frame::col4::DARKGRAY;
            else if (point_count == 120)
                return frame::col4::BLUE;
            else if (point_count == 400)
                return frame::col4::GREEN;
            else if (point_count == 800)
                return frame::col4::ORANGE;
            else if (point_count == 1600)
                return frame::col4::RED;
            return frame::col4::WHITE;
#else
            return frame::col4::DARKGRAY;
#endif
        };

        // TODO there is something wrong with the view, why it needs to be scaled down, this is wrong
        double semi_major_axis_pixel_size = view::get_world_to_pixel(semi_major_axis_world_size) / view::get_scale();

        if (semi_major_axis_pixel_size <= 1.0)
            return;
        
        auto& res = get_resolution(semi_major_axis_pixel_size);

        if (res.draw_id == frame::draw_buffer_id_invalid)
            res.draw_id = create_trajectory(points, res.point_count);

        frame::save_world_transform();
        frame::set_world_scale(frame::get_world_scale() * frame::vec2 { 1.0 / scale_factor, 1.0 / scale_factor });

        frame::draw_buffer(res.draw_id, get_color(res.point_count));

        //for (size_t i = 0; i < points.size(); i += points.size() / res.point_count)
        //    frame::draw_circle(points[i], scale_independent(1.5f), frame::col4::ORANGE);

        frame::restore_world_transform();
    }

    void init(kepler_orbit& orbit)
    {
        // special case for solar-system barycenter
        if (orbit.semi_major_axis == 0.0)
            return;

        auto semi_major_axis_world_size = commons::convert_AU_to_world_size(orbit.semi_major_axis);
        scale_factor = 1.0 / semi_major_axis_world_size;

        for (const auto& res : resmap)
        {
            resolutions.push_back(resolution{ res.semi_major_axis_pixel_size, res.point_count, frame::draw_buffer_id_invalid });
        }

        points = commons::draw_cast(orbit.get_orbit_points(max_orbit_points), scale_factor);

        static const double init_scale_limit_factor = 1000.0;

        for (auto& res : resolutions)
        {
            // TODO creating trajectories on the fly breaks drawing, what's the problem
            //if (semi_major_axis_world_size * init_scale_limit_factor < res.radius)
            //    break;

            res.draw_id = create_trajectory(points, res.point_count);
        }

        return;
    }

    std::vector<frame::vec2>& get_points()
    {
        return points;
    }

private:
   
    struct resolution
    {
        double radius = 0.0;
        int point_count = 0;
        frame::draw_buffer_id draw_id = frame::draw_buffer_id_invalid;
    };

    resolution& get_resolution(float semi_major_axis_pixel_size)
    {
        for (auto& res : resolutions)
        {
            // pick resolution, assume sorted, pick first which is bigger than draw size
            if (semi_major_axis_pixel_size < res.radius)
                return res;
        }

        //return {};
        return resolutions.back();
    }

    frame::draw_buffer_id create_trajectory(const std::vector<frame::vec2>& points, int point_count)
    {
        if (point_count > points.size())
            return frame::draw_buffer_id_invalid;

        size_t step = points.size() / point_count;

        std::vector<frame::vec2> trajectory;
        trajectory.reserve(point_count);

        for (size_t i = 0; i < points.size(); i += step)
            trajectory.push_back(points[i]);

        // add last point that will close the ellipse (if last point is not added in for loop)
        if ((points.size() - 1) % step != 0)
            trajectory.push_back(points.back());

        // TODO fix SG_USAGE_IMMUTABLE, problem is that we are re-creating buffer each time (in a case of immutable buffer)
        return frame::create_draw_buffer("polyline", { (float*)trajectory.data(), trajectory.size(), nullptr, 0 }, sg_primitive_type::SG_PRIMITIVETYPE_LINE_STRIP, sg_usage::SG_USAGE_DYNAMIC);
    }

    double scale_factor = 1.0;

    std::vector<resolution> resolutions;
    std::vector<frame::vec2> points;
};
