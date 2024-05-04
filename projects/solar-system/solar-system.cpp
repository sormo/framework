#include <framework.h>
#include <utils.h>
#include "unit.h"
#include "imgui.h"
#include "kepler_orbit.h"
#include <string>
#include <fstream>
#include "json.hpp"
#include "drawing_sg.h"
#include "trajectory_resolutions.h"
#include "commons.h"
#include "bodies_tree.h"
#include "quadtree.h"

using namespace frame;

double GRAVITATIONAL_CONSTANT = 0.8;

static double time_current = 0.0;
static double time_delta = 1.0 / 1000.0;
//static double time_delta = 0.0;

free_move_camera_config free_move_config;

bodies_tree bodies;
quadtree tree;

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

void step_bodies_tree(bodies_tree& data)
{
    for (auto& data : data.bodies)
        data.orbit.update_current_orbit_time_by_delta_time(time_delta);
}

void draw_ephemeris_trajectories(std::set<body_data*>& parents)
{
    std::function<void(body_data&)> draw_recursive = [&draw_recursive](body_data& data)
    {
        vec2 position = commons::draw_cast(data.orbit.position);

        if (std::isnan(position.x) || std::isnan(position.y))
            position = {};

        data.trajectory.draw(data.orbit.semi_major_axis);

        draw_circle(position, commons::scale_independent(5.0f), col4::RED);
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

void draw_ephemeris_names(std::set<body_data*>& parents)
{
    std::function<void(body_data&, std::vector<rectangle>&)> draw_recursive = [&draw_recursive](body_data& data, std::vector<rectangle>& parent_rects)
    {
        vec2 position = commons::draw_cast(data.orbit.position);
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

    for (auto parent : parents)
        draw_recursive(*parent, rectangles);
}

void draw_distance_legend()
{
    auto screen = frame::get_screen_size();

    const float line_size = screen.x / 8.0f;
    const float line_offset = screen.x / 20.0f;
    const float vertical_line_offset = commons::scale_independent(5.0f);
    const float line_thickness = commons::scale_independent(1.5f);

    vec2 left_point_screen = { screen.x - line_offset - line_size, screen.y - line_offset };
    vec2 right_point_screen = { screen.x - line_offset, screen.y - line_offset };

    vec2 left_point = frame::get_screen_to_world(left_point_screen);
    vec2 right_point = frame::get_screen_to_world(right_point_screen);

    frame::draw_line_solid_ex(left_point, right_point, line_thickness, frame::col4::WHITE);

    auto draw_vertical_line = [vertical_line_offset, line_thickness](const vec2& center)
    {
        frame::draw_line_solid_ex(frame::vec2(center.x, center.y - vertical_line_offset),
                                  frame::vec2(center.x, center.y + vertical_line_offset),
                                  line_thickness,
                                  frame::col4::WHITE);
    };

    draw_vertical_line(left_point);
    draw_vertical_line(right_point);

    vec2 center_point = left_point + (right_point - left_point) / 2.0f;

    double legend_size = (right_point - left_point).length();
    double value_au = unit::AU * legend_size / commons::DRAW_SIZE_FACTOR;
    double value_km = value_au * 1.496e+8;

    auto convert_value = [](double num) -> std::string
    {
        std::string text(256, '\0');
        if (num < 0.000'001 || num > 100'000.0)
            std::sprintf(text.data(), "%1.2e", num);
        else
            std::sprintf(text.data(), "%.3f", num);
        text.resize(strlen(text.data()));
        return text;
    };

    auto text_au = convert_value(value_au);
    auto text_km = convert_value(value_km);

    frame::draw_text((text_au + "AU").c_str(), center_point, 15.0f, col4::WHITE, text_align::bottom_middle);
    frame::draw_text((text_km + "km").c_str(), center_point - vec2(0.0f, commons::scale_independent(2.5f)), 15.0f, col4::WHITE, text_align::top_middle);
}

bodies_tree load_bodies_tree()
{
    std::vector<std::pair<std::string, std::string>> parents;
    bodies_tree result;

    auto read_file = [&parents, &result](const std::string& file_name)
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

            // inclination hack, we are showing this in 2d
            //inclination = 0.0;

            double AU = 1.495978707e11;
            // TODO G constant is used as free parameter to fixate orbits periods values while SemiMajor axis parameter is adjusted for the scene.
            double compensatedGConst = GRAVITATIONAL_CONSTANT / pow(AU / unit::AU, 3.0);

            kepler_orbit orbit;
            double attractor_mass = kepler_orbit::compute_mass(semi_major_axis * unit::AU * sqrt(1.0 - eccentricity * eccentricity), period, GRAVITATIONAL_CONSTANT);

            orbit.initialize(eccentricity, semi_major_axis * unit::AU, mean_anomaly, inclination, argument_of_periapsis, ascending_node_longitude, attractor_mass, GRAVITATIONAL_CONSTANT);

            trajectory_resolutions trajectory;
            trajectory.init(orbit);

            result.bodies.push_back({ body_name, std::move(orbit), nullptr, {}, trajectory });
            parents.push_back({ std::move(body_name), std::move(parent_name) });
        }
    };

    read_file("major-bodies.json");
    //read_file("test-bodies.json");
    //read_file("small-bodies-sbdb-100km.json");
    //read_file("small-bodies-sbdb-50km.json");

    // TODO assign parent-child relationships, can't add anything to this vector
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

void create_quadtree(float max_size, float min_size)
{
    // Creating world points is not neede. Tree is created only from parent bodies (which has parent Solar-System Barycenter)
    // because their trajectory points does not move. We don't care if childs has trajectory relative to parent as we don't add
    // their points to tree.
    //create_world_points_in_body_trajectories(bodies);
    tree.construct(frame::rectangle::from_min_max(-frame::vec2(max_size, max_size) / 2.0f, frame::vec2(max_size, max_size) / 2.0f), min_size, bodies);
}

void setup_quadtree()
{
    static const char* cache_filename = "quadtree_cache_100.cbor";
    static const float tree_min_size = 100.0f;

    std::ifstream cache_file_in(cache_filename, std::ios_base::binary);
    if (cache_file_in)
    {
        tree.deserialize(nlohmann::json::from_cbor(cache_file_in), bodies.bodies);
    }
    else
    {
        create_quadtree(100'000.0f, tree_min_size);

        auto json_tree = tree.serialize();

        std::vector<std::uint8_t> cbor_data = nlohmann::json::to_cbor(json_tree);
        std::ofstream cache_file_out(cache_filename, std::ios_base::binary);
        cache_file_out.write((const char*)cbor_data.data(), cbor_data.size());
    }
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

    frame::vec2 world_size{ 1'000'000.0f, 1'000'000.0f };

    free_move_config.min_size = { (float)commons::MIN_ZOOMED_SIZE, (float)commons::MIN_ZOOMED_SIZE };
    free_move_config.boundary = rectangle::from_center_size({ 400.0f, 300.0f }, world_size);

    setup_units();

    bodies = load_bodies_tree();

    setup_quadtree();
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

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Scale");
    ImGui::Text("%.2f %.2f ", get_world_scale().x, get_world_scale().y);

    //ImGui::TextColored(ImVec4(1, 1, 0, 1), "Jupiter Point Count");
    //ImGui::Text("%d", ellipse_point_count(5.0, frame::get_world_scale().x));

    ImGui::EndMainMenuBar();

    //ImGui::ShowDemoWindow();
}

void update_sg() {}

void update()
{
    // draw

    draw_debug_gui();

    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    //tree.draw_debug();

    auto parents = tree.query(frame::get_world_rectangle());

    draw_ephemeris_trajectories(parents);
    draw_ephemeris_names(parents);

    draw_distance_legend();

    // update

    step_bodies_tree(bodies);

    free_move_camera_update(free_move_config);

    time_current += time_delta;
}
