#include <framework.h>
#include <utils.h>
#include "unit.h"
#include "imgui.h"
#include "kepler_orbit.h"
#include <string>
#include <queue>
#include <fstream>
#include "json.hpp"
#include "drawing_sg.h"
#include "trajectory_resolutions.h"
#include "commons.h"
#include "bodies_tree.h"
#include "quadtree.h"
#include "camera.h"
#include "body_info.h"

using namespace frame;

static double time_current = 0.0;
static double time_delta = 1.0 / 1000.0;
//static double time_delta = 0.0;

bodies_tree bodies;
quadtree tree;
camera_type camera;
click_handler left_mouse_click = click_handler(frame::mouse_button::left);
body_color body_color_data;
body_info info;

void step_bodies_tree(bodies_tree& data)
{
    bodies.step(time_delta);
}

void draw_bodies_tree(bodies_tree& data)
{
    auto parents = tree.query(frame::get_world_rectangle());

    data.draw_trajectories(parents, body_color_data);
    data.draw_names(parents);
}

void draw_distance_legend()
{
    auto screen = frame::get_screen_size();

    const float line_size = screen.x / 8.0f;
    const float line_offset = screen.x / 20.0f;
    const float vertical_line_offset = commons::pixel_to_world(5.0f);
    const float line_thickness = commons::pixel_to_world(1.5f);

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
    double value_au = commons::convert_world_size_to_AU(legend_size);
    double value_km = commons::convert_AU_to_km(value_au);

    auto text_au = commons::convert_double_to_string(value_au);
    auto text_km = commons::convert_double_to_string(value_km);

    frame::draw_text_ex((text_au + "AU").c_str(), center_point, 15.0f, col4::WHITE, "roboto", text_align::bottom_middle);
    frame::draw_text_ex((text_km + "km").c_str(), center_point - vec2(0.0f, commons::pixel_to_world(2.5f)), 15.0f, col4::WHITE, "roboto", text_align::top_middle);
}

void load_bodies_tree()
{
    bodies.load({ "major-bodies.json", "small-bodies-sbdb-100km.json" });
    //bodies.load({ "major-bodies.json", "small-bodies-sbdb-50km.json" });
    //bodies.load({ "major-bodies.json" });
    //bodies.load({ "test-bodies.json" });
}

void create_quadtree(float max_size, float min_size)
{
    // Creating world points is not needed. Tree is created only from parent bodies (which has parent Solar-System Barycenter)
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

    unit::GRAVITATIONAL_CONSTANT = 4.0 * PI * PI;
    //GRAVITATIONAL_CONSTANT = 6.6743e-11 * (unit::meter * unit::meter * unit::meter) / (unit::kilogram * unit::second * unit::second);
}

void setup()
{
    auto size = get_screen_size();

    set_world_transform(translation(size / 2.0f) * scale({ 1.0f, -1.0f }));

    camera.setup();

    setup_units();

    load_bodies_tree();

    setup_quadtree();

    frame::load_font("roboto-medium", "fonts/roboto-medium.ttf");
    frame::load_font("roboto-black", "fonts/roboto-black.ttf");
    frame::load_font("roboto-bold", "fonts/roboto-bold.ttf");
    frame::load_font("roboto", "fonts/roboto.ttf");

    body_color_data.setup();
    info.setup();
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

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Translation");
    ImGui::Text("%.2f %.2f ", get_world_translation().x, get_world_translation().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Scale");
    ImGui::Text("%.2f %.2f ", get_world_scale().x, get_world_scale().y);

    //ImGui::TextColored(ImVec4(1, 1, 0, 1), "Jupiter Point Count");
    //ImGui::Text("%d", ellipse_point_count(5.0, frame::get_world_scale().x));

    ImGui::EndMainMenuBar();

    //ImGui::ShowDemoWindow();
}

body_node* get_clicked_body()
{
    static const float query_radius_pixels = 15.0f;
    static const float min_body_distance = 6.0f;

    auto mouse_position = frame::get_mouse_world_position();
    auto queried_bodies = bodies.query(mouse_position, commons::pixel_to_world(query_radius_pixels));
    if (queried_bodies.empty())
        return nullptr;

    if (queried_bodies.size() == 1)
        return queried_bodies[0];

    struct clicked_body
    {
        float distance_world_sqr;
        vec2 position;
        body_node* data;
    };
    std::vector<clicked_body> clicked_bodies;

    for (auto body : queried_bodies)
    {
        auto position = commons::draw_cast(body->get_absolute_position());
        clicked_bodies.push_back({ (position - mouse_position).length_sqr(), position, body });
    }

    // merge bodies which are too close together
    bool updated = true;
    while (updated)
    {
        std::set<size_t> delete_indices;
        updated = false;
        for (size_t i = 0; i < clicked_bodies.size(); i++)
        {
            for (size_t j = i + 1; j < clicked_bodies.size(); j++)
            {
                auto& body_i = clicked_bodies[i];
                auto& body_j = clicked_bodies[j];

                if ((body_i.position - body_j.position).length() < commons::pixel_to_world(min_body_distance))
                {
                    updated = true;
                    delete_indices.insert(body_i.data->radius > body_j.data->radius ? j : i);
                }
            }
        }

        // delete indices from last to first
        for (auto it = delete_indices.rbegin(); it != delete_indices.rend(); it++)
            clicked_bodies.erase(clicked_bodies.begin() + *it);
    }

    // pick the closest
    std::sort(std::begin(clicked_bodies), std::end(clicked_bodies), [](const clicked_body& a, const clicked_body& b) { return a.distance_world_sqr < b.distance_world_sqr; });

    return clicked_bodies.back().data;
}

void handle_left_click()
{
    if (!left_mouse_click.is_clicked())
        return;

    if (auto clicked_body = get_clicked_body())
    {
        info.set_body(clicked_body);
        camera.follow([clicked_body]() { return commons::draw_cast(clicked_body->get_absolute_position()); });
    }
    else
    {
        info.set_body(nullptr);
        camera.follow(nullptr);
    }
}

void update_sg() {}

void update()
{
    // draw

    //draw_debug_gui();

    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    //tree.draw_debug();

    draw_bodies_tree(bodies);

    draw_distance_legend();

    info.draw(body_color_data);

    // update

    step_bodies_tree(bodies);

    left_mouse_click.update();

    handle_left_click();

    camera.update();

    time_current += time_delta;
}
