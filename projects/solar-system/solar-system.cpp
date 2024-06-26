#include <framework.h>
#include <utils.h>
#include "unit.h"
#include "imgui.h"
#include "kepler_orbit.h"
#include <string>
#include <fstream>
#include "json.hpp"

using namespace frame;

double GRAVITATIONAL_CONSTANT = 0.8;
const double DRAW_SIZE_FACTOR = 200.0;
const double DRAW_VELOCITY_FACTOR = 13.0;

static double time_current = 0.0f;
static double time_delta = 1.0 / 1000.0;

free_move_camera_config free_move_config;

vec2 draw_cast(const vec3d& p)
{
    return { (float)p.x * DRAW_SIZE_FACTOR, (float)p.y * DRAW_SIZE_FACTOR };
}

float scale_independent(float s)
{
    return s / get_world_scale().x;
}

std::vector<vec2> draw_cast(const std::vector<vec3d>& data)
{
    std::vector<vec2> r;
    r.reserve(data.size());
    for (const auto& o : data)
        r.push_back(draw_cast(o));
    return r;
}

struct body_data
{
    std::string name;
    kepler_orbit orbit;

    body_data* parent = nullptr;
    std::vector<body_data*> childs;

    // trajectory
    std::vector<vec2> trajectory;
};

struct ephemeris_data
{
    std::vector<body_data> bodies;
    body_data* parent;
};

ephemeris_data ephem_data;

void step_ephemeris_data(ephemeris_data& data)
{
    for (auto& data : data.bodies)
        data.orbit.update_current_orbit_time_by_delta_time(time_delta);
}

void draw_ephemeris_trajectories(ephemeris_data& data)
{
    std::function<void(body_data&)> draw_recursive = [&draw_recursive](body_data& data)
        {
            vec2 position = draw_cast(data.orbit.position);

            if (std::isnan(position.x) || std::isnan(position.y))
                position = {};

            draw_bezier_polyline_ex(data.trajectory, scale_independent(1.0f), col4::GREEN);
            draw_circle(position, scale_independent(5.0f), col4::RED);
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

    draw_recursive(*data.parent);
}

void draw_ephemeris_names(ephemeris_data& data)
{
    std::function<void(body_data&, std::vector<rectangle>&)> draw_recursive = [&draw_recursive](body_data& data, std::vector<rectangle>& parent_rects)
    {
        vec2 position = draw_cast(data.orbit.position);
        rectangle rect = get_text_rectangle(data.name.c_str(), position, 15.0f, frame::text_align::bottom_left);

        bool do_draw = true;
        for (auto& parent_rect : parent_rects)
        {
            if (parent_rect.has_overlap(rect))
            {
                do_draw = false;
                break;
            }
        }

        if (do_draw)
            draw_text(data.name.c_str(), position, 15.0f, col4::GRAY, frame::text_align::bottom_left);

        if (!data.childs.empty())
        {
            parent_rects.push_back(rect);

            save_world_transform();

            set_world_translation(get_world_translation() + position * get_world_scale());

            for (auto& child : data.childs)
                draw_recursive(*child, parent_rects);

            restore_world_transform();

            parent_rects.pop_back();
        }
    };

    std::vector<rectangle> rectangles;
    draw_recursive(*data.parent, rectangles);
}

ephemeris_data load_ephemeris_data()
{
    std::ifstream f("ephemeris.json");
    nlohmann::json data = nlohmann::json::parse(f);

    struct orbit_with_parent
    {
        std::string parent;
        kepler_orbit orbit;
    };

    std::vector<std::pair<std::string, std::string>> parents;
    ephemeris_data result;

    for (const auto& orbit_data : data["OrbitsData"])
    {
        std::string body_name = orbit_data["BodyName"];
        std::string attractor_name = orbit_data["AttractorName"];
        double attractor_mass = orbit_data["AttractorMass"];
        double eccentricity = orbit_data["EC"];
        double inclination = orbit_data["IN"];
        double ascending_node_longitude = orbit_data["OM"];
        double argument_of_periapsis = orbit_data["W"];
        double mean_anomaly = orbit_data["MA"];
        double semi_major_axis = orbit_data["A"];

        // inclination hack, we are showing this in 2d
        inclination = 0.0;

        kepler_orbit orbit;
        orbit.initialize(eccentricity, semi_major_axis, mean_anomaly, inclination, argument_of_periapsis, ascending_node_longitude, attractor_mass * unit::kilogram, GRAVITATIONAL_CONSTANT);
        std::vector<vec2> trajectory = draw_cast(orbit.get_orbit_points());

        result.bodies.push_back({ body_name, std::move(orbit), nullptr, {}, std::move(trajectory) });
        parents.push_back({ std::move(body_name), std::move(attractor_name) });
    }

    // assign parent-child relationships, can't add anything to this vector
    auto get_body = [&result](const std::string& name) -> body_data*
        {
            for (auto& body : result.bodies)
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

    // assume single top-most parent
    for (auto& data : result.bodies)
    {
        if (!data.parent)
        {
            result.parent = &data;
            break;
        }
    }

    return result;
}

void setup_units()
{
    unit::set_base_meter((1.0 / 1.5e8) * 1e-3);
    unit::set_base_kilogram(1.0 / 2e30);
    unit::set_base_second(1e-7 / PI);

    GRAVITATIONAL_CONSTANT = 4.0 * PI * PI;
    //GRAVITATIONAL_CONSTANT = 6.6743e-11 * (unit::meter * unit::meter * unit::meter) / (unit::kilogram * unit::second * unit::second);
}


void setup()
{
    auto size = get_screen_size();

    set_world_transform(translation(size / 2.0f) * scale({ 1.0f, -1.0f }));

    free_move_config.min_size = { 1.0f, 1.0f };
    free_move_config.boundary = rectangle::from_center_size({ 400.0f, 300.0f }, { 100000.0f, 100000.0f });

    setup_units();

    ephem_data = load_ephemeris_data();
}

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_canvas = get_world_transform().inverted().transform_point(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", get_screen_size().x, get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", get_world_size().x, get_world_size().y);

    ImGui::EndMainMenuBar();

    //ImGui::ShowDemoWindow();
}

void update()
{
    // draw

    draw_debug_gui();

    draw_coordinate_lines(rgb(50, 50, 50));

    draw_ephemeris_trajectories(ephem_data);
    draw_ephemeris_names(ephem_data);

    // update

    //step_ephemeris_data(ephem_data);

    free_move_camera_update(free_move_config);

    time_current += time_delta;
}
