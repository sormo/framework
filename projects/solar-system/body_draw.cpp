#include "body_draw.h"
#include "commons.h"

using namespace frame;

void body_draw::initialize_instance_buffer(bodies_tree& bodies)
{
    points_instance_buffer = frame::create_draw_buffer_instanced("points",
                                                                 frame::create_mesh_circle_no_index(20),
                                                                 SG_PRIMITIVETYPE_TRIANGLE_STRIP,
                                                                 SG_USAGE_IMMUTABLE,
                                                                 bodies.bodies.size());

    size_t point_counter = 0;
    for (auto& body : bodies.bodies)
    {
        frame::update_draw_instance(points_instance_buffer,
                                    point_counter++,
                                    {},
                                    0.0f,
                                    { 7.0f, 7.0f },
                                    col4::WHITE);
    }
}

void body_draw::setup_bodies(bodies_tree& bodies)
{
    if (points_instance_buffer)
        frame::remove_buffer(points_instance_buffer);

    initialize_instance_buffer(bodies);
}

void body_draw::setup_models(commons::bodies_included_type type)
{
    body_drawer_model.fetch_models(type);
}

void body_draw::setup(body_color& cols)
{
    body_drawer_model.setup();
    body_drawer_shaded.setup();
    colors = &cols;

    planet_circle = frame::create_draw_buffer("planet-circle", frame::create_mesh_circle(60, 0.0f), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_IMMUTABLE);
}

void body_draw::draw_world_bodies(body_node* root)
{
    // TODO some problem with blending, points needs to be first
    if (settings.draw_points)
        draw_points({ root }, *colors);

    if (settings.draw_trajectories)
        draw_trajectories({ root }, *colors, root);

    // draw_points is doing instanced drawing of circles. Problem seems to be with modified opengl state
    // by sokol gfx. In particular sokol is calling glVertexAttribDivisor . This seems to make problem
    // when trying to draw with nanovg afterwards but for some reason only with webgl2. Don't know why.
    // Needed to add glVertexAttribDivisor(i, 0); to _sg_gl_reset_state_cache.
    sg_reset_state_cache();

    if (settings.draw_names)
        draw_names({ root }, false);
}

void draw_main_trajectory(body_node* main_body)
{
    auto screen_size = frame::get_screen_size();
    float max_pixel_size = 10'000; // some large number
    auto to = commons::draw_cast(main_body->orbit.velocity).normalized() * view::get_pixel_to_view(max_pixel_size);

    frame::draw_line_solid_ex(main_body->current_position.xy<float>(), to.xy<float>(), view::get_pixel_to_view(1.0f), frame::col4::DARKGRAY);
    frame::draw_line_solid_ex(main_body->current_position.xy<float>(), -to.xy<float>(), view::get_pixel_to_view(1.0f), frame::col4::DARKGRAY);
}

void body_draw::draw_main_body(body_node* main_body)
{
    // draw trajectory of main body
    if (settings.draw_trajectories)
    {
        draw_main_trajectory(main_body);

        draw_trajectories({ main_body->childs.begin(), main_body->childs.end() }, *colors, main_body);
    }

    frame::nanovg_flush();

    if (settings.draw_points)
        draw_points({ main_body }, *colors);

    sg_reset_state_cache();

    if (settings.draw_names)
        draw_names({ main_body }, true);
}

void body_draw::draw(body_node* body, bool is_root)
{
    if (is_root)
        draw_world_bodies(body);
    else
        draw_main_body(body);
}

void body_draw::draw_body(body_node& body, size_t& point_counter, body_color& colors, const rectangle& world_rectangle)
{
    auto position = body.current_position;

    float default_radius = view::get_pixel_to_world(3.5f);
    double body_radius = commons::convert_km_to_world_size(body.radius);

    auto color = body.group.empty() ? colors.get(body.type) : colors.get(body.group);

    if (body_radius > default_radius)
    {
        if (world_rectangle.contains(position.xy<float>()))
        {
            float radius = (float)view::get_world_to_view(body_radius);

            if (settings.shaded_planets)
            {
                if (!body_drawer_model.draw(body, radius, color))
                    body_drawer_shaded.draw(body, radius, color);
            }
            else
            {
                frame::draw_buffer(planet_circle, position, 0.0f, { 2.0f * radius, 2.0f * radius }, color);
            }
        }
    }
    else
    {
        auto screen_position = frame::get_world_to_screen(position.xy<float>());
        // increase depth by 10% to have point above trajectory
        float depth = position.z + 0.1f * std::fabs(position.z);

        frame::update_draw_instance(points_instance_buffer,
                                    point_counter++,
                                    vec3(screen_position, depth),
                                    color);
    }
}

void body_draw::draw_sun(body_node& body, size_t& point_counter, body_color& colors, const frame::rectangle& world_rectangle)
{
    if (settings.shaded_planets)
    {
        float default_radius = view::get_pixel_to_world(3.0f);
        double body_radius = commons::convert_km_to_world_size(body.radius);
        float radius = (float)view::get_world_to_view(body_radius);
        radius = std::max(default_radius, radius);

        auto color = body.group.empty() ? colors.get(body.type) : colors.get(body.group);

        body_drawer_shaded.draw(body, radius, color);
    }
    else
    {
        draw_body(body, point_counter, colors, world_rectangle);
    }
}

void body_draw::draw_points(const quadtree::query_result_type& parents, body_color& colors)
{
    auto world_rectangle = frame::get_world_rectangle();

    std::function<void(body_node&, size_t&)> draw_recursive = [this, &draw_recursive, &colors, world_rectangle](body_node& data, size_t& point_counter)
    {
        if (is_body_node_skip(data))
            return;

        if (data.type != body_type::barycenter)
        {
            if (data.type == body_type::star)
                draw_sun(data, point_counter, colors, world_rectangle);
            else
                draw_body(data, point_counter, colors, world_rectangle);
        }

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

// we will highlight orbit of clicked body
body_node* get_highlight_body()
{
    if (!state.clicked_body)
        return nullptr;
    // for major body of barycentric system highlight trajectory of barycenter
    if (state.clicked_body->parent && state.clicked_body->parent->type == body_type::barycenter && state.clicked_body->system.is_major_body)
        return state.clicked_body->parent;
    return state.clicked_body;
}

trajectory_resolutions::color_t get_color_type(body_node* body, body_node* highlight_body)
{
    if (body == highlight_body)
        return trajectory_resolutions::color_t::highlight;

    // HACK diminish trajectory of sun
    if (body->type == body_type::star)
        return trajectory_resolutions::color_t::diminish;

    return trajectory_resolutions::color_t::normal;
}

void body_draw::draw_trajectories(const quadtree::query_result_type& parents, body_color& colors, body_node* stationary_body)
{
    body_node* highlight_body = get_highlight_body();

    std::function<void(body_node&, const vec3&)> draw_recursive = [this, &draw_recursive, &colors, stationary_body, highlight_body](body_node& data, const vec3& parent_position)
    {
        if (is_body_node_skip(data))
            return;

        data.trajectory.draw(parent_position, commons::convert_AU_to_world_size(data.orbit.semi_major_axis), data.parent == stationary_body, get_color_type(&data, highlight_body));

        if (data.childs.empty())
            return;

        for (auto& child : data.childs)
            draw_recursive(*child, data.current_position);
    };

    for (auto parent : parents)
    {
        draw_recursive(*parent, parent->parent ? parent->parent->current_position : vec3{});
    }

    trajectory_resolutions::draw_cache.flush();
}

void body_draw::draw_names(const quadtree::query_result_type& parents, bool is_view)
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

        auto rounded = std::roundf((float)std::min(body_diameter / 4.0f, max_char_size));

        return std::min(frame::get_screen_size().y / 2.0f, rounded);
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

        auto position = body->current_position;

        if (body->type != body_type::barycenter)
        {
            auto computed_font_size = get_computed_font_size(body);
            if (computed_font_size > commons::NAME_FONT_SIZE)
            {
                draw_text_ex(body->name.c_str(), position.xy<float>(), computed_font_size, col4::DARKGRAY, "roboto-bold", text_align::middle_middle, 3.0f);
                draw_text_ex(body->name.c_str(), position.xy<float>(), computed_font_size-1.0f, col4::LIGHTGRAY, "roboto-bold", text_align::middle_middle);
            }
            else
            {
                auto rect = get_text_rectangle(body, position.xy<float>());

                if (!check_overlap(rect))
                {
                    rectangles.push_back(std::move(rect));
                    if (is_view)
                        draw_text_ex2(body->name.c_str(), position, commons::NAME_FONT_SIZE, col4::LIGHTGRAY, "roboto-bold", text_align::bottom_left);
                    else
                        draw_text_ex(body->name.c_str(), position.xy<float>(), commons::NAME_FONT_SIZE, col4::LIGHTGRAY, "roboto-bold", text_align::bottom_left);
                }
            }
        }

        for (auto* child : body->childs)
            Q.push(child);
    }
}
