#pragma once
#include "commons.h"

struct trajectory_resolutions
{
    void draw(double semi_major_axis)
    {
        auto get_color = [](int point_count)
        {
            if (point_count < 200)
                return frame::col4::DARKGRAY;
            else if (point_count < 1000)
                return frame::col4::BLUE;
            else if (point_count < 4000)
                return frame::col4::GREEN;
            else if (point_count < 8000)
                return frame::col4::ORANGE;
            return frame::col4::RED;
        };

        double world_scale = frame::get_world_scale().x;
        double draw_size = semi_major_axis * world_scale * commons::DRAW_SIZE_FACTOR;

        if (draw_size <= 1.0)
            return;
        
        auto& res = get_resolution(semi_major_axis, world_scale);

        if (res.draw_id == frame::draw_buffer_id_invalid)
            return;

        frame::draw_buffer(res.draw_id, {}, 0.0f, { 1.0f, 1.0f }, get_color(res.point_count));

        //for (size_t i = 0; i < points.size(); i += points.size() / res.point_count)
        //    frame::draw_circle(points[i], scale_independent(1.5f), frame::col4::ORANGE);
    }

    void init(kepler_orbit& orbit)
    {
        // special case for solar-system barycenter
        if (orbit.semi_major_axis == 0.0)
            return;

        static const double max_point_distance = 180.0;

        const double scales[] = { 0.1, 1.0, 5.0, 20.0, 200.0 };
        for (size_t i = 0; i < 5; i++)
        {
            double radius = orbit.semi_major_axis * scales[i] * commons::DRAW_SIZE_FACTOR;
            int point_count = 2.0 * frame::PI * radius / max_point_distance;

            point_count = std::max(60, point_count);
            point_count = std::min(100'000, point_count);

            if (!resolutions.empty() && resolutions.back().point_count == point_count)
                resolutions.back().radius = radius;
            else
                resolutions.push_back(resolution{ radius, point_count, frame::draw_buffer_id_invalid });
        }

        // initialize all points (for maximum resolution)
        int max_point_count = get_resolution(orbit.semi_major_axis, get_maximum_scale()).point_count;
        points = commons::draw_cast(orbit.get_orbit_points(max_point_count));

        if (points.empty())
            return;

        for (auto& res : resolutions)
        {
            if (res.point_count)
                res.draw_id = create_trajectory(points, res.point_count);
        }
    }

    std::vector<frame::vec2>& get_points()
    {
        return points;
    }

private:
    float get_maximum_scale()
    {
        return std::min(frame::get_screen_size().x, frame::get_screen_size().y) / commons::MIN_ZOOMED_SIZE;
    }
    
    struct resolution
    {
        double radius = 0.0;
        int point_count = 0;
        frame::draw_buffer_id draw_id = frame::draw_buffer_id_invalid;
    };

    resolution get_resolution(double semi_major_axis, float scale)
    {
        double radius = semi_major_axis * scale * commons::DRAW_SIZE_FACTOR;

        for (auto& res : resolutions)
        {
            // pick resolution, assume sorted, pick first which is bigger than draw size
            if (radius < res.radius)
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
        if (points.size() % step != 0)
            trajectory.push_back(points.back());

        // TODO fix SG_USAGE_IMMUTABLE, problem is that we are re-creating buffer each time (in a case of immutable buffer)
        return frame::create_draw_buffer("polyline", { (float*)trajectory.data(), trajectory.size(), nullptr, 0 }, sg_primitive_type::SG_PRIMITIVETYPE_LINE_STRIP, sg_usage::SG_USAGE_DYNAMIC);
    }

    std::vector<resolution> resolutions;
    std::vector<frame::vec2> points;
};
