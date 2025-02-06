#include "body_draw_shaded.h"
#include "commons.h"
#include "body_draw_planet.glsl.h"

using namespace frame;

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
    pipeline_desc.layout.attrs[ATTR_body_draw_planet_position].format = SG_VERTEXFORMAT_FLOAT2;
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
    auto draw_axis_half = [](const body_node& body, float radius, bool is_lower)
    {
        static const float axis_length = 5.0f;
        static const float axit_width = 0.02f;

        auto direction = is_lower ? -body.rotation_axis : body.rotation_axis;
        auto orientation = frame::mat4::look_at(body.rotation_axis);

        // if we are drawing upper half, draw only part that is visible above the sphere
        // if drawing lower half, just draw whole half, it will be clipped by planet's billboard
        if (direction.z > 0.0f)
        {
            auto length = radius * axis_length / 2.0f - radius;
            auto scale = mat4::scaling({ radius * axit_width, length, radius * axit_width });
            auto model = mat4::translation(direction * (radius * axis_length / 2.0f + radius) / 2.0f + body.current_position) * orientation * scale;
            frame::draw_cylinder(create_world_projection_view() * model, col4::RGBf(0.5f, 0.5f, 0.5f), sshapes_shading::flat, -vec3(body.get_absolute_position()), model);
        }
        else
        {
            // make it slightly smaller to not go through planet billboard
            auto scale = mat4::scaling({ radius * axit_width, radius * axis_length * 0.98f / 2.0f, radius * axit_width });
            auto model = mat4::translation(direction * axis_length * radius / 4.0f + body.current_position) * orientation * scale;
            frame::draw_cylinder(create_world_projection_view() * model, col4::RGBf(0.5f, 0.5f, 0.5f), sshapes_shading::flat, -vec3(body.get_absolute_position()), model);
        }
    };

    auto draw_planet = [this](body_node& body, float radius, const frame::col4& color)
    {
        sg_apply_pipeline(pip_planet);
        sg_apply_bindings(&bind_planet);

        enum draw_type
        {
            planet = 0,
            sun = 1
        };

        fs_params_body_draw_common_t params_fs_common = {};

        if (body.type == body_type::star)
        {
            auto params_vs_sun = create_sun_vs_params(body.current_position, radius);
            auto params_fs_sun = create_sun_fs_params();

            sg_apply_uniforms(UB_vs_params_body_draw_common, SG_RANGE(params_vs_sun));
            sg_apply_uniforms(UB_fs_params_body_draw_sun, SG_RANGE(params_fs_sun));

            params_fs_common.draw_type = draw_type::sun;
        }
        else
        {
            auto params_vs_planet = create_planet_vs_params(body.current_position, radius);
            auto params_fs_planet = create_planet_fs_params(color, body.get_absolute_position());

            sg_apply_uniforms(UB_vs_params_body_draw_common, SG_RANGE(params_vs_planet));
            sg_apply_uniforms(UB_fs_params_body_draw_planet, SG_RANGE(params_fs_planet));

            params_fs_common.draw_type = draw_type::planet;
        }

        sg_apply_uniforms(UB_fs_params_body_draw_common, SG_RANGE(params_fs_common));

        sg_draw(0, 4, 1);
    };

    if (body.rotation_period && settings.show_rotation_axis)
    {
        // draw halfs of axis first half is the one behind planet (based on direction whether is forward (to camera))
        draw_axis_half(body, radius, body.rotation_axis.z > 0.0f);
    }

    draw_planet(body, radius, color);

    if (body.rotation_period && settings.show_rotation_axis)
    {
        //frame::draw_sphere(create_world_mvp(body.current_position, 0.0f, { 2.0f * radius }), col4::BLANK);
        draw_axis_half(body, radius, !(body.rotation_axis.z > 0.0f));
    }
}
