#include "body_draw_model.h"
#include <sokol_app.h>
#include "body_draw_model.glsl.h"
#include "commons.h"

using namespace frame;

extern commons::settings_data settings;

void body_draw_model::setup_pip()
{
    sg_pipeline_desc pip_desc = {};
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    pip_desc.shader = sg_make_shader(body_draw_model_shader_desc(sg_query_backend()));;

    pip_desc.layout.attrs[ATTR_body_draw_model_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_body_draw_model_vs_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

    pip_desc.colors[0].blend.enabled = true;
    pip_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    pip_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pip_desc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
    pip_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
    pip_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
    pip_desc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;

    pip_model = sg_make_pipeline(pip_desc);
}

bool body_draw_model::load_model(body_node& body)
{
    if (!models_zip)
        return false;

    auto model_name = body.name + ".obj";

    auto data = frame::get_zip_file(*models_zip, model_name);
    if (data.empty())
        return false;

    auto model = frame::load_obj_flat(data);
    if (!model)
        return false;

    models_cache[body.name] = *model;

    return true;
}

bool body_draw_model::load_buffer(body_node& body)
{
    if (!models_cache.count(body.name))
    {
        if (!load_model(body))
            return false;
    }

    auto& model = models_cache[body.name];

    sg_buffer_desc buffer_desc_vert = {};
    buffer_desc_vert.data = sg_range{ (void*)model.vertices.data(), model.vertices.size() * sizeof(vertex_t) };
    buffer_cache[body.name] = sg_make_buffer(&buffer_desc_vert);

    return true;
}

bool body_draw_model::setup_bind(body_node& body)
{
    if (!available_models.count(body.name + ".obj"))
        return false;

    if (!buffer_cache.count(body.name))
    {
        if (!load_buffer(body))
            return false;
    }

    bind_model.vertex_buffers[0] = buffer_cache[body.name];
    element_count = models_cache[body.name].vertices.size();

    bound_body = body.name;

    return true;
}

void body_draw_model::setup(const std::vector<char>& models_zip_data)
{
    setup_pip();

    models_zip = frame::open_zip(models_zip_data);

    if (models_zip)
    {
        auto zip_files = frame::list_zip_files(*models_zip);
        for (auto& z : zip_files)
        {
            auto data = frame::get_zip_file(*models_zip, z);

            available_models.insert(std::move(z));
        }
    }
}

body_draw_model_fs_params_t create_fs_params(const frame::col4& color, const vec3d& orbit_position)
{
    body_draw_model_fs_params_t result = {};

    result.light_color = vec3(1.0f, 1.0f, 1.0f);
    result.object_color = color.to_vec3();
    result.ambient_strength = 0.001f;
    result.specular_strength = 0.1f;

    result.view_position = vec3(0.0f, 0.0f, 5.0f);
    result.light_position = -vec3(orbit_position.normalized() * 10.0f);

    return result;
}

HMM_Mat4 create_rotation()
{
    static const float speed = 0.005f;

    static float rx = 0.0f;
    static float ry = 0.0f;

    HMM_Mat4 rxm = HMM_Rotate_RH(rx, HMM_Vec3{ 1.0f, 0.0f, 0.0f });
    HMM_Mat4 rym = HMM_Rotate_RH(ry, HMM_Vec3{ 0.0f, 1.0f, 0.0f });

    const float t = (float)(sapp_frame_duration() * 60.0);
    rx += speed * t; ry += (speed / 2.0f) * t;

    return HMM_MulM4(rxm, rym);
}

std::pair<HMM_Mat4, HMM_Mat4> create_world_mvp(const vec3& position, float radius)
{
    static const float mesh_size = 2.0f;

    float scale_factor = radius * 2.0f / mesh_size;

    auto scale = HMM_Scale({ scale_factor, scale_factor, scale_factor });
    auto rotate = create_rotation();
    auto translate = HMM_Translate({ position.x, position.y, position.z });
    HMM_Mat4 model_matrix = HMM_MulM4(HMM_MulM4(translate, rotate), scale);

    return { create_projection_view_matrix(), model_matrix };
}

body_draw_model_vs_params_t create_vs_params(const vec3d& position, float radius)
{
    body_draw_model_vs_params_t result = {};

    std::tie(result.projection_view, result.model) = create_world_mvp(position, radius);

    return result;
}

bool body_draw_model::draw(body_node& body, float radius, const frame::col4& color)
{
    if (!bound_body || *bound_body != body.name)
    {
        if (!setup_bind(body))
            return false;
    }

    sg_apply_pipeline(pip_model);
    sg_apply_bindings(&bind_model);

    body_draw_model_fs_params_t params_fs = create_fs_params(color, body.get_absolute_position());
    body_draw_model_vs_params_t params_vs = create_vs_params(body.current_position, radius);

    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_body_draw_model_vs_params, SG_RANGE(params_vs));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_body_draw_model_fs_params, SG_RANGE(params_fs));

    sg_draw(0, element_count, 1);

    return true;
}
