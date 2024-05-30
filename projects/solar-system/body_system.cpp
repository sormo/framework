#include "body_system.h"
#include <fstream>

using namespace frame;

extern commons::settings_data settings;

static double time_current = 0.0; // in days

void body_system::setup_colors(std::map<std::string, std::vector<char>>& files)
{
    auto& data = files["colors.json"];
    data.push_back('\0');
    body_color_data.setup(data.data());
}

void body_system::setup_info(std::map<std::string, std::vector<char>>& files)
{
    info.setup(files["icons/body_icons.zip"], body_color_data);
}

void setup_units()
{
    unit::set_base_meter((1.0 / 1.5e8) * 1e-3);
    unit::set_base_kilogram(1.0 / 2e30);
    unit::set_base_second(1e-7 / PI);

    unit::GRAVITATIONAL_CONSTANT = 4.0 * PI * PI;
    //GRAVITATIONAL_CONSTANT = 6.6743e-11 * (unit::meter * unit::meter * unit::meter) / (unit::kilogram * unit::second * unit::second);
}

void body_system::load_bodies_tree(std::vector<std::vector<char>*> files)
{
    bodies.clear();

    std::vector<const char*> bodies_jsons;
    for (auto data : files)
    {
        data->push_back('\0');
        bodies_jsons.push_back(data->data());
    }
    bodies.load(bodies_jsons);
}

void body_system::setup_bodies(commons::bodies_included_type type)
{
    settings.body_system_initializing = true;

    const char* cache_file = nullptr;
    const char* small_bodies_file = nullptr;

    if (type == commons::bodies_included_type::more_than_10)
        std::tie(small_bodies_file, cache_file) = std::pair{ "small-bodies-sbdb-10km.json", "cache/quadtree_cache_10.cbor" };
    else if (type == commons::bodies_included_type::more_than_50)
        std::tie(small_bodies_file, cache_file) = std::pair{ "small-bodies-sbdb-50km.json", "cache/quadtree_cache_50.cbor" };
    else
        std::tie(small_bodies_file, cache_file) = std::pair{ "small-bodies-sbdb-100km.json", "cache/quadtree_cache_100.cbor" };

    //fetch_files({ "test-bodies.json", small_bodies_file, cache_file }, [this, cache_file, small_bodies_file](std::map<std::string, std::vector<char>> files)
    fetch_files({ "major-bodies.json", small_bodies_file, cache_file }, [this, cache_file, small_bodies_file](std::map<std::string, std::vector<char>> files)
    {
        //load_bodies_tree({ &files["test-bodies.json"] });
        load_bodies_tree({ &files["major-bodies.json"], &files[small_bodies_file]});
        setup_quadtree(cache_file, files[cache_file]);

        settings.body_system_initializing = false;
    });
}

void body_system::create_quadtree(float max_size, float min_size)
{
    // Creating world points is not needed. Tree is created only from parent bodies (which has parent Solar-System Barycenter)
    // because their trajectory points does not move. We don't care if childs has trajectory relative to parent as we don't add
    // their points to tree.
    //create_world_points_in_body_trajectories(bodies);
    tree.construct(frame::rectangle::from_min_max(-frame::vec2(max_size, max_size) / 2.0f, frame::vec2(max_size, max_size) / 2.0f), min_size, bodies);
}

void body_system::setup_quadtree(const char* cache_filename, const std::vector<char>& cache_file_data)
{
    static const float tree_min_size = 100.0f;

    if (cache_file_data.empty())
    {
        create_quadtree(100'000.0f, tree_min_size);

        auto json_tree = tree.serialize();

        std::vector<std::uint8_t> cbor_data = nlohmann::json::to_cbor(json_tree);
        std::ofstream cache_file_out(cache_filename, std::ios_base::binary);
        cache_file_out.write((const char*)cbor_data.data(), cbor_data.size());
    }
    else
    {
        tree.deserialize(nlohmann::json::from_cbor(cache_file_data), bodies.bodies);
    }
}

void body_system::step_bodies_tree()
{
    if (settings.step_time)
        bodies.step(settings.step_speed);
}

void body_system::draw_bodies_tree()
{
    auto parents = tree.query(frame::get_world_rectangle());

    if (settings.draw_trajectories)
        bodies.draw_trajectories(parents, body_color_data);

    if (settings.draw_points)
        bodies.draw_points(parents, body_color_data);

    if (settings.draw_names)
        bodies.draw_names(parents);
}

void body_system::draw_distance_legend()
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

    frame::draw_text_ex((text_au + "AU").c_str(), center_point, 15.0f, col4::LIGHTGRAY, "roboto", text_align::bottom_middle);
    frame::draw_text_ex((text_km + "km").c_str(), center_point - vec2(0.0f, commons::pixel_to_world(2.5f)), 15.0f, col4::LIGHTGRAY, "roboto", text_align::top_middle);
}

void body_system::draw_current_time()
{
    static const float offset = 20.0f;

    time_t t = 1514764800; // 2018.01.01 00:00:00
    t += (time_current * 86400.0); // convert days to seconds

    auto tm = *std::gmtime(&t);

    char time_str[256] = {};
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S UTC", &tm);

    frame::save_world_transform();
    frame::set_world_transform(frame::identity());

    frame::draw_text_ex(time_str, { offset, get_screen_size().y - offset }, 15.0f, col4::LIGHTGRAY, "roboto", frame::text_align::bottom_left);

    frame::restore_world_transform();
}

body_node* body_system::query(const frame::vec2& world_position, float radius_in_pixels)
{
    static const float min_body_distance = 6.0f;

    auto queried_bodies = bodies.query(world_position, commons::pixel_to_world(radius_in_pixels));
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
        clicked_bodies.push_back({ (position - world_position).length_sqr(), position, body });
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

void body_system::set_info(body_node* body)
{
    info.set_body(body);
}

void body_system::setup()
{
    setup_units();

    fetch_files({ "colors.json", "icons/body_icons.zip" }, [this](std::map<std::string, std::vector<char>> files)
    {
        setup_colors(files);
        setup_info(files);
    });

    setup_bodies(settings.bodies_included);
}

void body_system::draw()
{
    draw_bodies_tree();
    info.draw();
    //tree.draw_debug();
    draw_distance_legend();
    draw_current_time();
}

void body_system::update()
{
    step_bodies_tree();

    if (settings.step_time)
        time_current += settings.step_speed;
}
