#include "bodies_tree.h"
#include "framework.h"
#include "commons.h"
#include "unit.h"
#include "view.h"
#include <json.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <queue>
#include <set>

using namespace frame;

extern commons::settings_data settings;

static const float name_font_size = 15.0f;

std::vector<body_node*> bodies_tree::query(const frame::vec2& query_point, float query_radius)
{
    std::vector<body_node*> result;

    float query_radius_sqr = query_radius * query_radius;

    std::function<void(vec2, body_node&)> query_recursive = [this, &query_recursive, &result, query_point, query_radius_sqr](vec2 parent_position, body_node& body)
    {
        auto position = commons::draw_cast(body.orbit.position) + parent_position;

        if (!is_barycenter(body) && (position - query_point).length_sqr() <= query_radius_sqr)
            result.push_back(&body);

        for (auto& child : body.childs)
            query_recursive(position, *child);
    };

    query_recursive({}, *parent);

    return result;
}

double get_time_offset(std::string time)
{
    // this is 0 time: 2018-01-01
    if (time == "2018-01-01")
        return 0.0;

    int year = std::stoi(time.substr(0, 4));
    int month = std::stoi(time.substr(5, 2));
    int day = std::stoi(time.substr(8, 2));

    auto make_time_point = [](int year, int month, int day)
    {
        std::tm tm = {};
        tm.tm_year = year - 1900; // std::tm's year is since 1900
        tm.tm_mon = month - 1;    // std::tm's month is 0-based
        tm.tm_mday = day;
        return std::chrono::system_clock::from_time_t(std::mktime(&tm));
    };

    return std::chrono::duration_cast<std::chrono::hours>(make_time_point(2018,1,1) - make_time_point(year,month,day)).count() / 24.0;
}

void bodies_tree::load(std::vector<const char*> json_datas)
{
    save_world_transform();
    set_world_transform(frame::identity());

    std::vector<std::pair<std::string, std::string>> parents;

    for (auto json_data : json_datas)
    {
        nlohmann::json data = nlohmann::json::parse(json_data);

        for (const auto& orbit_data : data)
        {
            std::string body_name = orbit_data["name"];
            body_type body_type = get_body_type_from_string(orbit_data["type"]);
            std::string body_group = orbit_data["group"];
            std::string parent_name = orbit_data["parent_name"];
            double period = orbit_data["period"]; // days
            double eccentricity = orbit_data["EC"];
            double inclination = deg_to_rad(orbit_data["IN"]);
            double ascending_node_longitude = deg_to_rad(orbit_data["OM"]);
            double argument_of_periapsis = deg_to_rad(orbit_data["W"]);
            double mean_anomaly = deg_to_rad(orbit_data["MA"]);
            double semi_major_axis = orbit_data["A"]; // AU
            double radius = orbit_data["radius"]; // km

            double mass = 0.0;
            if (orbit_data.contains("mass"))
                mass = orbit_data["mass"]; // kg
            else if (orbit_data.contains("gm"))
                mass = (double)orbit_data["gm"] * 6.6743e-20; // TODO possibly add mass to sbdb data or vice versa

            double density = 0.0;
            if (orbit_data.contains("density"))
                density = orbit_data["density"];

            std::string dimensions_str;
            if (orbit_data.contains("dimensions"))
            {
                dimensions_str = commons::convert_double_to_string(orbit_data["dimensions"][0]);
                for (size_t i = 0; i < orbit_data["dimensions"].size(); i++)
                    dimensions_str += "x" + commons::convert_double_to_string(orbit_data["dimensions"][i]);
            }

            // inclination hack, we are showing this in 2d
            if (settings.disable_inclination)
                inclination = 0.0;

            double AU = 1.495978707e11;
            // TODO G constant is used as free parameter to fixate orbits periods values while SemiMajor axis parameter is adjusted for the scene.
            double compensatedGConst = unit::GRAVITATIONAL_CONSTANT / pow(AU / unit::AU, 3.0);

            kepler_orbit orbit;
            double attractor_mass = kepler_orbit::compute_mass(semi_major_axis * unit::AU * sqrt(1.0 - eccentricity * eccentricity), period, unit::GRAVITATIONAL_CONSTANT);

            orbit.initialize(eccentricity, semi_major_axis * unit::AU, mean_anomaly, inclination, argument_of_periapsis, ascending_node_longitude, attractor_mass, unit::GRAVITATIONAL_CONSTANT);

            orbit.update_initial_mean_anomaly_with_time_offset(get_time_offset(orbit_data.contains("time") ? orbit_data["time"] : "2018-01-01"));

            body_node node;
            node.name = body_name;
            node.type = body_type;
            node.group = body_group;
            node.radius = radius;
            node.period = period;
            node.mass = mass;
            node.density = density;
            node.dimensions_str = dimensions_str;
            node.inclination = inclination;
            node.orbit = std::move(orbit);

            node.trajectory.init(node.orbit);
            node.name_text_rectangle = get_text_rectangle_ex(body_name.c_str(), {}, name_font_size, "roboto-bold");

            bodies.push_back(std::move(node));
            parents.push_back({ std::move(body_name), std::move(parent_name) });
        }
    }

    initialize_instance_buffer();

    // TODO assign parent-child relationships, can't add anything to this vector
    auto get_body = [this](const std::string& name) -> body_node*
    {
        for (auto& body : bodies)
            if (body.name == name)
                return &body;
        return nullptr;
    };
    for (const auto& [child, parent] : parents)
    {
        body_node* child_body = get_body(child);
        body_node* parent_body = get_body(parent);

        if (!child_body || !parent_body)
            continue;

        child_body->parent = parent_body;
        parent_body->childs.push_back(child_body);
    }

    // sort childs by radius decreasing
    for (auto& body : bodies)
        std::sort(std::begin(body.childs), std::end(body.childs), [](const body_node* a, const body_node* b) { return a->radius > b->radius; });

    // assume single top-most parent
    for (auto& data : bodies)
    {
        if (!data.parent)
        {
            parent = &data;
            break;
        }
    }

    // set major bodies
    for (auto& data : bodies)
    {
        if (data.type != body_type::barycenter)
            continue;
        body_node* major_node = nullptr;
        for (auto& child : data.childs)
        {
            if (major_node == nullptr || child->mass > major_node->mass)
                major_node = child;
        }
        if (major_node)
            major_node->system.is_major_body = true;

        data.system.major_body = major_node;
    }

    restore_world_transform();
}

// skip child bodies which has too small semi-major axis
// - do not skip major bodies in a case of barycentric system
// - do not skip barycenter
bool bodies_tree::is_body_node_skip(const body_node& node)
{
    double semi_major_axis_pixels = view::get_world_to_pixel(commons::convert_AU_to_world_size(node.orbit.semi_major_axis));

    return node.type != body_type::barycenter && !node.system.is_major_body && semi_major_axis_pixels < 2.0f;
}

void bodies_tree::draw_names(const quadtree::query_result_type& parents)
{
    std::vector<rectangle> rectangles;
    auto check_overlap = [&rectangles](const frame::rectangle& rect)
    {
        for (const auto& rect_overlap : rectangles)
        {
            if (rect_overlap.has_overlap(rect))
                return true;
        }
        return false;
    };

    auto get_computed_font_size = [this](const body_node* body)
    {
        auto body_radius = view::get_world_to_pixel(commons::convert_km_to_world_size(body->radius));
        auto body_diameter = body_radius * 2.0f;
        auto max_char_size = body_diameter / body->name.size();

        return (float)std::min(body_diameter / 4.0f, max_char_size);
    };

    auto get_text_rectangle = [](const body_node* body, const vec2& position) -> frame::rectangle
    {
        auto result = body->name_text_rectangle;
        result.min /= get_world_scale();
        result.max /= get_world_scale();
        result.min += position;
        result.max += position;

        // the rectangle has top_left align (means [0,0] is top left) and we need bottom_left
        vec2 align_modif(0.0f, result.size().y);

        std::swap(result.min.y, result.max.y);

        result.min -= align_modif;
        result.max -= align_modif;

        return result;
    };

    std::queue<body_node*> Q; // BFS

    for (auto parent : parents)
        Q.push(parent);

    while (!Q.empty())
    {
        body_node* body = Q.front();
        Q.pop();

        if (is_body_node_skip(*body))
            continue;

        vec2 position = body->current_position;

        if (!is_barycenter(*body))
        {
            auto computed_font_size = get_computed_font_size(body);
            if (computed_font_size > name_font_size)
            {
                // TODO looks like rendering text with larger font size is super slow
                draw_text_ex(body->name.c_str(), position, computed_font_size, col4::LIGHTGRAY, "roboto-bold", text_align::middle_middle);
            }
            else
            {
                auto rect = get_text_rectangle(body, position);

                if (!check_overlap(rect))
                {
                    rectangles.push_back(std::move(rect));
                    draw_text_ex(body->name.c_str(), position, name_font_size, col4::LIGHTGRAY, "roboto-bold", text_align::bottom_left);
                }
            }
        }

        for (auto* child : body->childs)
            Q.push(child);
    }
}

void bodies_tree::update_current_positions(const quadtree::query_result_type& parents)
{
    std::function<void(vec2, body_node&)> update_recursive = [this, &update_recursive](vec2 parent_position, body_node& data)
    {
        if (is_body_node_skip(data))
            return;

        // here we use the fact that we have already position relative to main body, so only scale is needed
        data.current_position = commons::draw_cast(data.orbit.position * view::get_scale()) + parent_position;

        if (!data.childs.empty())
        {
            for (auto& child : data.childs)
                update_recursive(data.current_position, *child);
        }
    };

    for (auto parent : parents)
        update_recursive({}, *parent);
}

void bodies_tree::initialize_instance_buffer()
{
    points_instance_buffer = frame::create_draw_buffer_instanced("points",
                                                                 frame::create_mesh_circle_no_index(20),
                                                                 SG_PRIMITIVETYPE_TRIANGLE_STRIP,
                                                                 SG_USAGE_IMMUTABLE,
                                                                 bodies.size());

    size_t point_counter = 0;
    for (auto& body : bodies)
    {
        frame::update_draw_instance(points_instance_buffer,
                                    point_counter++,
                                    {},
                                    0.0f,
                                    { 7.0f, 7.0f },
                                    col4::WHITE);
    }
}

void bodies_tree::draw_points(const quadtree::query_result_type& parents, body_color& colors)
{
    std::function<void(body_node&, size_t&)> draw_recursive = [this, &draw_recursive, &colors](body_node& data, size_t& point_counter)
    {
        if (is_body_node_skip(data))
            return;

        vec2 position = data.current_position;

        float default_radius = view::get_pixel_to_world(3.5f);
        double body_radius = commons::convert_km_to_world_size(data.radius);

        auto color = data.group.empty() ? colors.get(data.type) : colors.get(data.group);

        if (body_radius > default_radius)
            draw_circle(position, (float)view::get_world_to_view(body_radius), color);
        else
            frame::update_draw_instance(points_instance_buffer,
                                        point_counter++,
                                        frame::get_world_to_screen(position),
                                        color);

        if (!data.childs.empty())
        {
            for (auto& child : data.childs)
                draw_recursive(*child, point_counter);
        }
    };

    size_t point_count = 0;
    for (auto parent : parents)
        draw_recursive(*parent, point_count);

    if (point_count != 0)
    {
        frame::save_world_transform();
        frame::set_world_transform(frame::identity());

        frame::draw_buffer_instanced(points_instance_buffer, point_count);

        frame::restore_world_transform();
    }
}

void bodies_tree::draw_trajectories(const quadtree::query_result_type& parents, body_color& colors, body_node* stationary_body)
{
    stationary_body = stationary_body == nullptr ? parent : stationary_body;

    std::function<void(body_node&, const vec2&)> draw_recursive = [this, &draw_recursive, &colors, stationary_body](body_node& data, const vec2& parent_position)
    {
        if (is_body_node_skip(data))
            return;

        data.trajectory.draw(parent_position, commons::convert_AU_to_world_size(data.orbit.semi_major_axis), data.parent == stationary_body);
        
        if (data.childs.empty())
            return;

        for (auto& child : data.childs)
            draw_recursive(*child, data.current_position);
    };

    for (auto parent : parents)
    {
        draw_recursive(*parent, parent->parent ? parent->parent->current_position : vec2{});
    }

    trajectory_resolutions::draw_cache.flush();
}

void bodies_tree::step(double time_delta)
{
    for (auto& data : bodies)
        data.orbit.update_current_orbit_time_by_delta_time(time_delta);
}

bool bodies_tree::is_barycenter(const body_node& body)
{
    return body.type == body_type::barycenter;
}

// TODO not needed 
void create_world_points_in_body_trajectories(bodies_tree& data)
{
    std::function<void(body_node&, vec2)> create_recursive = [&create_recursive](body_node& data, vec2 parent_position)
    {
        for (auto& p : data.trajectory.get_points())
            p += parent_position;

        vec2 position = commons::draw_cast(data.orbit.position);
        for (auto& child : data.childs)
            create_recursive(*child, parent_position + position);
    };

    create_recursive(*data.parent, {});
}

void bodies_tree::clear()
{
    bodies.clear();
    parent = nullptr;
}

void bodies_tree::draw(const quadtree::query_result_type& parents, body_color& colors)
{
    update_current_positions(parents);

    if (settings.draw_trajectories)
        draw_trajectories(parents, colors);

    if (settings.draw_points)
        draw_points(parents, colors);

    if (settings.draw_names)
        draw_names(parents);
}

void bodies_tree::draw_lagrangians(body_node* body)
{
    // if this is major body and barycenter does not have parent, we will skip lagrangians
    if (body->system.is_major_body && !body->parent->parent)
        return;


    std::map<lagrangian, vec2> lagrangians;
    for (auto lagr : { lagrangian::l1, lagrangian::l2, lagrangian::l3, lagrangian::l4, lagrangian::l5 })
    {
        auto position = commons::draw_cast(body->get_lagrangian(lagr) * view::get_scale());

        position += body->current_position;

        lagrangians[lagr] = position;

        draw_circle(position, view::get_pixel_to_view(3.0f), col4::DARKGRAY);
        draw_text_ex(get_lagrangian_to_string(lagr).c_str(), position, name_font_size, col4::LIGHTGRAY, "roboto-bold", text_align::bottom_left);
    }

    minor_system system(*body);

    auto thickness = view::get_pixel_to_view(0.5f);

    draw_line_solid_ex(lagrangians[lagrangian::l2], lagrangians[lagrangian::l3], thickness, frame::col4::DARKGRAY);
    draw_line_solid_ex(system.minor->current_position, lagrangians[lagrangian::l4], thickness, frame::col4::DARKGRAY);
    draw_line_solid_ex(system.major->current_position, lagrangians[lagrangian::l4], thickness, frame::col4::DARKGRAY);
    draw_line_solid_ex(system.minor->current_position, lagrangians[lagrangian::l5], thickness, frame::col4::DARKGRAY);
    draw_line_solid_ex(system.major->current_position, lagrangians[lagrangian::l5], thickness, frame::col4::DARKGRAY);
}
