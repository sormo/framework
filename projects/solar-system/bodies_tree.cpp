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

std::vector<body_node*> bodies_tree::query(const frame::vec2& query_point, float query_radius)
{
    std::vector<body_node*> result;

    float query_radius_sqr = query_radius * query_radius;

    std::function<void(vec3, body_node&)> query_recursive = [this, &query_recursive, &result, query_point, query_radius_sqr](vec3 parent_position, body_node& body)
    {
        auto position = commons::draw_cast(body.orbit.position) + parent_position;

        if (!is_barycenter(body) && (position.xy<float>() - query_point).length_sqr() <= query_radius_sqr)
            result.push_back(&body);

        for (auto& child : body.childs)
            query_recursive(position, *child);
    };

    query_recursive({}, *parent);

    return result;
}

// TODO hardcoded init time, should be in commons 
double get_time_offset(std::string time)
{
    // this is 0 time: 2018-01-01
    if (time == "2024-03-31")
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

    return std::chrono::duration_cast<std::chrono::hours>(make_time_point(2024,3,31) - make_time_point(year,month,day)).count() / 24.0;
}

static vec3 get_rotation_axis(double lambda, double beta)
{
    vec3 rotation_axis;
    rotation_axis.x = cosf(beta) * cosf(lambda);
    rotation_axis.y = cosf(beta) * sinf(lambda);
    rotation_axis.z = sinf(beta);
    return rotation_axis;
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
                for (size_t i = 1; i < orbit_data["dimensions"].size(); i++)
                    dimensions_str += "\n" + commons::convert_double_to_string(orbit_data["dimensions"][i]);
            }

            double rotation_period = 0.0;
            vec3 rotation_axis;
            double rotation_jd0 = 0.0;
            if (orbit_data.contains("rotation"))
            {
                if (orbit_data["rotation"].contains("period"))
                {
                    if (orbit_data["rotation"]["period"] == "sync")
                        rotation_period = -period;
                    else
                        rotation_period = orbit_data["rotation"]["period"];
                }
                if (orbit_data["rotation"].contains("long"))
                    rotation_axis = get_rotation_axis(deg_to_rad(orbit_data["rotation"]["long"]), deg_to_rad(orbit_data["rotation"]["lat"]));
                if (orbit_data["rotation"].contains("jd0"))
                    rotation_jd0 = orbit_data["rotation"]["jd0"];
            }

            std::optional<double> temp_min, temp_mean, temp_max;
            if (orbit_data.contains("temperature"))
            {
                if (orbit_data["temperature"].contains("min"))
                    temp_min = orbit_data["temperature"]["min"];
                if (orbit_data["temperature"].contains("mean"))
                    temp_mean = orbit_data["temperature"]["mean"];
                if (orbit_data["temperature"].contains("max"))
                    temp_max = orbit_data["temperature"]["max"];
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

            // if rotation axis is not specified, set it to be perpendicular to orbit plane
            if (rotation_axis == vec3{0.0f})
                rotation_axis = orbit.semi_major_axis_basis.cross(orbit.semi_minor_axis_basis);

            auto time_offset = get_time_offset(orbit_data.contains("time") ? orbit_data["time"] : "2024-03-31");
            orbit.set_current_orbit_time(time_offset);

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

            node.rotation_axis = rotation_axis;
            node.rotation_period = rotation_period;
            node.rotation_jd0 = rotation_jd0;

            if (orbit_data.contains("mesh"))
                node.mesh = orbit_data["mesh"];

            node.trajectory.init(node.orbit);
            node.name_text_rectangle = get_text_rectangle_ex(body_name.c_str(), {}, commons::NAME_FONT_SIZE, "roboto-bold");
            node.world_radius = commons::convert_km_to_world_size(node.radius);

            node.temperature.min = temp_min;
            node.temperature.mean = temp_mean;
            node.temperature.max = temp_max;

            bodies.push_back(std::move(node));

            //for (const auto& p : parents)
            //    if (p.first == body_name)
            //        __debugbreak();

            parents.push_back({ std::move(body_name), std::move(parent_name) });
        }
    }

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

void bodies_tree::update_current_positions(const quadtree::query_result_type& parents)
{
    std::function<void(vec3, body_node&)> update_recursive = [this, &update_recursive](vec3 parent_position, body_node& data)
    {
        if (is_body_node_skip(data))
            return;

        // here we use the fact that we have already position relative to main body, so only scale is needed
        auto position = vec3{ data.orbit.position.x * view::get_scale(), data.orbit.position.y * view::get_scale(), data.orbit.position.z };
        data.current_position = commons::draw_cast(position) + parent_position;

        if (!data.childs.empty())
        {
            for (auto& child : data.childs)
                update_recursive(data.current_position, *child);
        }
    };

    for (auto parent : parents)
        update_recursive({}, *parent);
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

void bodies_tree::clear()
{
    bodies.clear();
    parent = nullptr;
}
