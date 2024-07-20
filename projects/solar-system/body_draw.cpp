#include "body_draw.h"
#include "body_draw_planet.glsl.h"

using namespace frame;

void body_draw::setup()
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

    result.atmosphere_radius_relative = 0.11f;
    result.atmosphere_density = 0.8f;
    result.atmosphere_color = col4::RGBf(1.0f, 0.9f, 0.7f).to_vec3();

    return result;
}

static vs_params_body_draw_planet_t create_planet_vs_params(const vec2& position, const float radius)
{
    vs_params_body_draw_planet_t result = {};

    // scale should be set to twice the diameter, because shader has half of the place for planet and half is for atmosphere
    float scale_size = 4.0f * radius;

    result.mvp = frame::create_world_mvp(position, 0.0f, vec2{ scale_size, scale_size });

    return result;
}

void body_draw::draw(body_node& body, float radius, const frame::col4& color)
{
    sg_apply_pipeline(pip_planet);
    sg_apply_bindings(&bind_planet);

    auto params_vs = create_planet_vs_params(body.current_position, radius);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params_body_draw_planet, SG_RANGE(params_vs));

    auto params_fs = create_planet_fs_params(color, body.get_absolute_position());
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params_body_draw_planet, SG_RANGE(params_fs));

    sg_draw(0, 4, 1);
}
