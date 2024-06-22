#include "trajectory_resolutions.h"
#include "commons.h"

trajectories_draw_cache trajectory_resolutions::draw_cache;

void trajectories_draw_cache::draw(frame::draw_buffer_id id, frame::mat3&& transform, frame::col4&& color)
{
    ids.push_back(id);
    transforms.push_back(std::move(transform));
    colors.push_back(std::move(color));
}

void trajectories_draw_cache::flush()
{
    frame::draw_buffers(ids, transforms, colors);

    ids.clear();
    transforms.clear();
}

frame::mat3 trajectory_resolutions::get_transform(const frame::vec2& position, bool has_stationary_parent)
{
    if (has_stationary_parent)
    {
        if (!cached_transform)
            cached_transform = frame::translation(position) * frame::scale(frame::vec2{ 1.0 / scale_factor, 1.0 / scale_factor });
        return *cached_transform;
    }
    else
    {
        cached_transform = {};

        return frame::translation(position) * frame::scale(frame::vec2{ 1.0 / scale_factor, 1.0 / scale_factor });
    }
}

void trajectory_resolutions::draw(const frame::vec2& world_translation, double semi_major_axis_world_size, bool has_stationary_parent)
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
    double semi_major_axis_pixel_size = double(view::get_world_to_pixel(semi_major_axis_world_size)) / view::get_scale();

    if (semi_major_axis_pixel_size <= 1.0)
        return;

    auto& res = get_resolution(semi_major_axis_pixel_size);

    if (res.draw_id == frame::draw_buffer_id_invalid)
        res.draw_id = create_trajectory(points, res.point_count);

    auto transform = get_transform(world_translation, has_stationary_parent);

    draw_cache.draw(res.draw_id, std::move(transform), get_color(res.point_count));

    //for (size_t i = 0; i < points.size(); i += points.size() / res.point_count)
    //    frame::draw_circle(points[i], scale_independent(1.5f), frame::col4::ORANGE);
}

void trajectory_resolutions::init(kepler_orbit& orbit)
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

std::vector<frame::vec2>& trajectory_resolutions::get_points()
{
    return points;
}

trajectory_resolutions::resolution& trajectory_resolutions::get_resolution(double semi_major_axis_pixel_size)
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

frame::draw_buffer_id trajectory_resolutions::create_trajectory(const std::vector<frame::vec2>& points, int point_count)
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
