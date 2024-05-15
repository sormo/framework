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

vec3d body_node::get_absolute_position()
{
    vec3d result = orbit.position;
    body_node* current_parent = parent;
    while (current_parent)
    {
        result += current_parent->orbit.position;
        current_parent = current_parent->parent;
    }
    return result;
}

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
            //inclination = 0.0;

            double AU = 1.495978707e11;
            // TODO G constant is used as free parameter to fixate orbits periods values while SemiMajor axis parameter is adjusted for the scene.
            double compensatedGConst = unit::GRAVITATIONAL_CONSTANT / pow(AU / unit::AU, 3.0);

            kepler_orbit orbit;
            double attractor_mass = kepler_orbit::compute_mass(semi_major_axis * unit::AU * sqrt(1.0 - eccentricity * eccentricity), period, unit::GRAVITATIONAL_CONSTANT);

            orbit.initialize(eccentricity, semi_major_axis * unit::AU, mean_anomaly, inclination, argument_of_periapsis, ascending_node_longitude, attractor_mass, unit::GRAVITATIONAL_CONSTANT);

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

            bodies.push_back(std::move(node));
            parents.push_back({ std::move(body_name), std::move(parent_name) });
        }
    };

    for (const auto& file_path : files)
        read_file(file_path.c_str());

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
            major_node->is_major_body = true;
    }
}

// skip child bodies which has too small semi-major axis
// - do not skip major bodies in a case of barycentric system
// - do not skip barycenter
bool is_body_node_skip(const body_node& node)
{
    double semi_major_axis = commons::convert_AU_to_world_size(node.orbit.semi_major_axis) * frame::get_world_scale().x;

    return node.type != body_type::barycenter && !node.is_major_body && semi_major_axis < 2.0f;
}

void bodies_tree::draw_names(std::set<body_node*>& parents)
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

    auto get_draw_data = [](const body_node* body) -> std::pair<float, frame::text_align>
    {
        double body_radius = commons::convert_km_to_world_size(body->radius) * frame::get_world_scale().x;
        double body_diameter = body_radius * 2.0;
        double max_char_size = body_diameter / body->name.size();
        auto computed_font_size = (float)std::min(body_diameter / 4.0, max_char_size);

        return { std::max(15.0f, computed_font_size), (computed_font_size > 15.0f ? frame::text_align::middle_middle : frame::text_align::bottom_left) };
    };

    std::queue<body_node*> Q; // BFS
    std::map<body_node*, vec2> parent_translations;

    parent_translations[nullptr] = vec2();

    Q.push(parent);

    while (!Q.empty())
    {
        body_node* body = Q.front();
        Q.pop();

        if (is_body_node_skip(*body))
            continue;

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

void bodies_tree::draw_trajectories(std::set<body_node*>& parents, body_color& colors)
{
    std::function<void(body_node&)> draw_recursive = [&draw_recursive, &colors](body_node& data)
    {
        if (is_body_node_skip(data))
            return;

        vec2 position = commons::draw_cast(data.orbit.position);

        if (std::isnan(position.x) || std::isnan(position.y))
            position = {};

        data.trajectory.draw(data.orbit.semi_major_axis);

        float default_radius = commons::pixel_to_world(3.5f);
        draw_circle(position, default_radius, data.group.empty() ? colors.get(data.type) : colors.get(data.group));

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