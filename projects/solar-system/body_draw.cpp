#include "body_draw.h"
#include "commons.h"
#include "body_draw_planet.glsl.h"

using namespace frame;

extern commons::settings_data settings;

void body_draw_shaded::setup()
{
    struct vertex_t
    {
        float x, y;
    };

    vertex_t vertices[] =
    {
         0.5f,  0.5f,
         0.5f, -0.5f,
        -0.5f, -0.5f,
        -0.5f,  0.5f
    };

    sg_buffer_desc buffer_desc_vert = {};
    buffer_desc_vert.data = SG_RANGE(vertices);
    buffer_desc_vert.label = "planet-vertices";
    bind_planet.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    uint16_t indices[] = { 0, 1, 3, 2 };
    sg_buffer_desc buffer_desc_index = {};
    buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
    buffer_desc_index.data = SG_RANGE(indices);
    buffer_desc_index.label = "planet-indices";
    bind_planet.index_buffer = sg_make_buffer(&buffer_desc_index);

    sg_shader shd = sg_make_shader(body_draw_planet_shader_desc(sg_query_backend()));

    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    pipeline_desc.layout.attrs[ATTR_body_draw_planet_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
    pipeline_desc.shader = shd;
    pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    pipeline_desc.label = "planet-pipeline";

    pipeline_desc.colors[0].blend.enabled = true;
    pipeline_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    pipeline_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_desc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
    pipeline_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
    pipeline_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
    pipeline_desc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;

    pipeline_desc.depth.write_enabled = true;
    pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

    pip_planet = sg_make_pipeline(&pipeline_desc);
}

fs_params_body_draw_planet_t create_planet_fs_params(const col4& color, const vec3d& orbit_position)
{
    fs_params_body_draw_planet_t result = {};
    result.light_power = 40.0f;
    result.diffuse_color = color.to_vec3();
    result.specular_color = vec3(0.1f);
    result.shininess = 16.0f;

    result.camera_position = { 0.0f, 0.0f, 1.0f };
    result.light_position = -vec3(orbit_position.normalized() * 6.0f);
    result.planet_radius = 0.25f;

    result.atmosphere_radius_relative = 0.11f;
    result.atmosphere_density = 0.8f;
    result.atmosphere_color = col4::RGBf(1.0f, 0.9f, 0.7f).to_vec3();

    return result;
}

fs_params_body_draw_sun_t create_sun_fs_params()
{
    fs_params_body_draw_sun_t result = {};
    result.sun_radius = 0.136f;
    result.sun_intensity = 2.0f;

    return result;
}

static vs_params_body_draw_common_t create_planet_vs_params(const vec3& position, const float radius)
{
    vs_params_body_draw_common_t result = {};

    // scale should be set to twice the diameter, because shader has half of the place for planet and half is for atmosphere
    float scale_size = 4.0f * radius;

    result.mvp = frame::create_world_mvp(position, 0.0f, vec2{ scale_size, scale_size });

    return result;
}

static vs_params_body_draw_common_t create_sun_vs_params(const vec3& position, const float radius)
{
    vs_params_body_draw_common_t result = {};

    float scale_size = 23.688f * radius;

    result.mvp = frame::create_world_mvp(position, 0.0f, vec2{ scale_size, scale_size });

    return result;
}

void body_draw_shaded::draw(body_node& body, float radius, const frame::col4& color)
{
    sg_apply_pipeline(pip_planet);
    sg_apply_bindings(&bind_planet);

    enum draw_type
    {
        planet = 0,
        sun = 1
    };

    fs_params_body_draw_common_t params_fs_common = {};

    // just quick test
    if (body.type == body_type::star)
    {
        auto params_vs_sun = create_sun_vs_params(body.current_position, radius);
        auto params_fs_sun = create_sun_fs_params();

        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params_body_draw_common, SG_RANGE(params_vs_sun));
        sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params_body_draw_sun, SG_RANGE(params_fs_sun));

        params_fs_common.draw_type = draw_type::sun;
    }
    else
    {
        auto params_vs_planet = create_planet_vs_params(body.current_position, radius);
        auto params_fs_planet = create_planet_fs_params(color, body.get_absolute_position());

        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params_body_draw_common, SG_RANGE(params_vs_planet));
        sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params_body_draw_planet, SG_RANGE(params_fs_planet));

        params_fs_common.draw_type = draw_type::planet;
    }

    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params_body_draw_common, SG_RANGE(params_fs_common));

    sg_draw(0, 4, 1);
}

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

void body_draw::setup(body_color& cols)
{
    body_drawer.setup();
    colors = &cols;

    planet_circle = frame::create_draw_buffer("planet-circle", frame::create_mesh_circle(60, 0.0f), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_IMMUTABLE);
}

void body_draw::draw_world_bodies(body_node* root)
{
    if (settings.draw_trajectories)
        draw_trajectories({ root }, *colors, root);

    if (settings.draw_points)
        draw_points({ root }, *colors);

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

void body_draw::draw_points(const quadtree::query_result_type& parents, body_color& colors)
{
    std::function<void(body_node&, size_t&)> draw_recursive = [this, &draw_recursive, &colors](body_node& data, size_t& point_counter)
    {
        if (is_body_node_skip(data))
            return;

        auto position = data.current_position;

        float default_radius = view::get_pixel_to_world(3.5f);
        double body_radius = commons::convert_km_to_world_size(data.radius);

        auto color = data.group.empty() ? colors.get(data.type) : colors.get(data.group);

        if (body_radius > default_radius)
        {
            float radius = (float)view::get_world_to_view(body_radius);

            if (settings.shaded_planets)
                body_drawer.draw(data, radius, color);
            else
                frame::draw_buffer(planet_circle, position, 0.0f, { 2.0f * radius, 2.0f * radius }, color);
        }
        else if (data.type != body_type::barycenter)
        {
            auto screen_position = frame::get_world_to_screen(position.xy<float>());
            // increase depth by 10% to have point above trajectory
            float depth = position.z + 0.1f * std::fabs(position.z);

            frame::update_draw_instance(points_instance_buffer,
                                        point_counter++,
                                        vec3(screen_position, depth),
                                        color);
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

body_node* get_clicked_body_orbit_highlight()
{
    if (!state.clicked_body)
        return nullptr;
    // for major body of barycentric system highlight trajectory of barycenter
    if (state.clicked_body->parent && state.clicked_body->parent->type == body_type::barycenter && state.clicked_body->system.is_major_body)
        return state.clicked_body->parent;
    return state.clicked_body;
}

void body_draw::draw_trajectories(const quadtree::query_result_type& parents, body_color& colors, body_node* stationary_body)
{
    body_node* orbit_highlight = get_clicked_body_orbit_highlight();

    std::function<void(body_node&, const vec3&)> draw_recursive = [this, &draw_recursive, &colors, stationary_body, orbit_highlight](body_node& data, const vec3& parent_position)
    {
        if (is_body_node_skip(data))
            return;

        data.trajectory.draw(parent_position, commons::convert_AU_to_world_size(data.orbit.semi_major_axis), data.parent == stationary_body, &data == orbit_highlight);

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
