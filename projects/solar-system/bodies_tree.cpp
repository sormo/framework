#include "bodies_tree.h"
#include "framework.h"
#include "commons.h"
#include "unit.h"
#include <json.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <queue>
#include <set>

using namespace frame;

vec3d body_data::get_absolute_position()
{
    vec3d result = orbit.position;
    body_data* current_parent = parent;
    while (current_parent)
    {
        result += current_parent->orbit.position;
        current_parent = current_parent->parent;
    }
    return result;
}

std::vector<body_data*> bodies_tree::query(const frame::vec2& query_point, float query_radius)
{
    std::vector<body_data*> result;

    float query_radius_sqr = query_radius * query_radius;

    std::function<void(vec2, body_data&)> query_recursive = [this, &query_recursive, &result, query_point, query_radius_sqr](vec2 parent_position, body_data& body)
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

void bodies_tree::load(const std::vector<std::string>& files)
{
    std::vector<std::pair<std::string, std::string>> parents;

    auto read_file = [this, &parents](const std::string& file_name)
    {
        std::ifstream f(file_name);
        nlohmann::json data = nlohmann::json::parse(f);

        for (const auto& orbit_data : data)
        {
            std::string body_name = orbit_data["name"];
            std::string parent_name = orbit_data["parent_name"];
            double period = orbit_data["period"]; // kg
            double eccentricity = orbit_data["EC"];
            double inclination = deg_to_rad(orbit_data["IN"]);
            double ascending_node_longitude = deg_to_rad(orbit_data["OM"]);
            double argument_of_periapsis = deg_to_rad(orbit_data["W"]);
            double mean_anomaly = deg_to_rad(orbit_data["MA"]);
            double semi_major_axis = orbit_data["A"]; // AU
            double radius = orbit_data["radius"];

            // inclination hack, we are showing this in 2d
            //inclination = 0.0;

            double AU = 1.495978707e11;
            // TODO G constant is used as free parameter to fixate orbits periods values while SemiMajor axis parameter is adjusted for the scene.
            double compensatedGConst = unit::GRAVITATIONAL_CONSTANT / pow(AU / unit::AU, 3.0);

            kepler_orbit orbit;
            double attractor_mass = kepler_orbit::compute_mass(semi_major_axis * unit::AU * sqrt(1.0 - eccentricity * eccentricity), period, unit::GRAVITATIONAL_CONSTANT);

            orbit.initialize(eccentricity, semi_major_axis * unit::AU, mean_anomaly, inclination, argument_of_periapsis, ascending_node_longitude, attractor_mass, unit::GRAVITATIONAL_CONSTANT);

            trajectory_resolutions trajectory;
            trajectory.init(orbit);

            bodies.push_back({ body_name, std::move(orbit), radius, nullptr,{}, trajectory });
            parents.push_back({ std::move(body_name), std::move(parent_name) });
        }
    };

    for (const auto& file_path : files)
        read_file(file_path.c_str());

    // TODO assign parent-child relationships, can't add anything to this vector
    auto get_body = [this](const std::string& name) -> body_data*
    {
        for (auto& body : bodies)
            if (body.name == name)
                return &body;
        return nullptr;
    };
    for (const auto& [child, parent] : parents)
    {
        body_data* child_body = get_body(child);
        body_data* parent_body = get_body(parent);

        if (!child_body || !parent_body)
            continue;

        child_body->parent = parent_body;
        parent_body->childs.push_back(child_body);
    }

    // sort childs by radius decreasing
    for (auto& body : bodies)
        std::sort(std::begin(body.childs), std::end(body.childs), [](const body_data* a, const body_data* b) { return a->radius > b->radius; });

    // assume single top-most parent
    for (auto& data : bodies)
    {
        if (!data.parent)
        {
            parent = &data;
            break;
        }
    }
}

void bodies_tree::draw_names(std::set<body_data*>& parents)
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

    auto get_draw_data = [](const body_data* body) -> std::pair<float, frame::text_align>
    {
        double body_radius = commons::convert_km_to_world_size(body->radius) * frame::get_world_scale().x;
        double body_diameter = body_radius * 2.0;
        double max_char_size = body_diameter / body->name.size();
        auto computed_font_size = (float)std::min(body_diameter / 4.0, max_char_size);

        return { std::max(15.0f, computed_font_size), (computed_font_size > 15.0f ? frame::text_align::middle_middle : frame::text_align::bottom_left) };
    };

    std::queue<body_data*> Q; // BFS
    std::map<body_data*, vec2> parent_translations;

    parent_translations[nullptr] = vec2();

    Q.push(parent);

    while (!Q.empty())
    {
        body_data* body = Q.front();
        Q.pop();

        vec2 position = parent_translations[body->parent] + commons::draw_cast(body->orbit.position);

        if (!is_barycenter(*body))
        {
            auto [text_size, text_align] = get_draw_data(body);

            auto rect = get_text_rectangle_ex(body->name.c_str(), position, text_size, "roboto-bold", text_align);

            if (!check_overlap(rect))
            {
                rectangles.push_back(rect);
                // TODO looks like rendering text with larger font size is super slow
                draw_text_ex(body->name.c_str(), position, text_size, col4::LIGHTGRAY, "roboto-bold", text_align);
            }
        }

        parent_translations[body] = position;
        for (auto* child : body->childs)
            Q.push(child);
    }
}

void bodies_tree::draw_trajectories(std::set<body_data*>& parents)
{
    std::function<void(body_data&)> draw_recursive = [&draw_recursive](body_data& data)
    {
        vec2 position = commons::draw_cast(data.orbit.position);

        if (std::isnan(position.x) || std::isnan(position.y))
            position = {};

        data.trajectory.draw(data.orbit.semi_major_axis);

        float default_radius = commons::pixel_to_world(3.5f);
        draw_circle(position, default_radius, col4::RGB(137, 187, 215, 255));

        double body_radius = commons::convert_km_to_world_size(data.radius);
        if (body_radius > default_radius)
            draw_circle(position, body_radius, col4::ORANGE);
        //draw_text(data.name.c_str(), position, 15.0f, col4::GRAY, frame::text_align::bottom_left);

        if (!data.childs.empty())
        {
            save_world_transform();

            set_world_translation(get_world_translation() + position * get_world_scale());

            for (auto& child : data.childs)
                draw_recursive(*child);

            restore_world_transform();
        }
    };

    for (auto parent : parents)
        draw_recursive(*parent);
}

void bodies_tree::step(double time_delta)
{
    for (auto& data : bodies)
        data.orbit.update_current_orbit_time_by_delta_time(time_delta);
}

bool bodies_tree::is_barycenter(const body_data& body)
{
    return body.name.find("Barycenter") != std::string::npos;
}

// TODO not needed 
void create_world_points_in_body_trajectories(bodies_tree& data)
{
    std::function<void(body_data&, vec2)> create_recursive = [&create_recursive](body_data& data, vec2 parent_position)
    {
        for (auto& p : data.trajectory.get_points())
            p += parent_position;

        vec2 position = commons::draw_cast(data.orbit.position);
        for (auto& child : data.childs)
            create_recursive(*child, parent_position + position);
    };

    create_recursive(*data.parent, {});
}