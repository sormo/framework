#include "body_draw_model.h"
#include <sokol_app.h>
#include "body_draw_model.glsl.h"
#include "commons.h"
#include "drawing_sg.h"

using namespace frame;

extern commons::settings_data settings;

body_draw_model::body_draw_model()
    : buffer_model(SG_USAGE_DYNAMIC, SG_BUFFERTYPE_VERTEXBUFFER)
{

}

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

std::optional<model_t> body_draw_model::load_model(body_node& body)
{
    model_zip* zip = nullptr;

    if (models_moons && models_moons->available_models.count(body.mesh))
        zip = &(*models_moons);
    if (models_100 && models_100->available_models.count(body.mesh))
        zip = &(*models_100);
    else if (models_50 && models_50->available_models.count(body.mesh))
        zip = &(*models_50);
    else if (models_10 && models_10->available_models.count(body.mesh))
        zip = &(*models_10);

    if (!zip)
        return {};

    auto data = frame::get_zip_file(zip->zip, body.mesh);
    if (data.empty())
        return {};

    return frame::load_glb_flat(data);
}

bool body_draw_model::load_buffer(body_node& body)
{
    auto model = load_model(body);
    if (!model)
        return false;

    auto range_id = buffer_model.append((const char*)model->vertices.data(), model->vertices.size() * sizeof(vertex_t));

    buffer_cache[body.name] = { range_id, model->vertices.size() };

    return true;
}

bool body_draw_model::setup_bind(body_node& body)
{
    if (body.mesh.empty())
        return false;

    if (!buffer_cache.count(body.name))
    {
        if (!load_buffer(body))
            return false;
    }

    const auto& model_data = buffer_cache[body.name];

    buffer_model.apply(model_data.range_id, bind_model);

    element_count = model_data.element_count;

    bound_body = body.name;

    return true;
}

void body_draw_model::setup()
{
    setup_pip();
}

void body_draw_model::fetch_models(commons::bodies_included_type type)
{
    std::vector<std::string> files;
    std::vector<std::optional<model_zip>*> zips;

    files.push_back("models/moons.zip");
    zips.push_back(&models_moons);

    switch (type)
    {
    case commons::bodies_included_type::more_than_10:
        if (!models_10)
        {
            files.push_back("models/models_10.zip");
            zips.push_back(&models_10);
        }
    case commons::bodies_included_type::more_than_50:
        if (!models_50)
        {
            files.push_back("models/models_50.zip");
            zips.push_back(&models_50);
        }
    case commons::bodies_included_type::more_than_100:
        if (!models_100)
        {
            files.push_back("models/models_100.zip");
            zips.push_back(&models_100);
        }
    }

    fetch_files(files, [files, zips](std::map<std::string, std::vector<char>> files_data)
    {
        auto create_model_zip = [](std::vector<char> data) -> std::optional<model_zip>
        {
            auto zip = frame::open_zip(data);
            if (!zip)
                return {};

            model_zip result;
            result.zip = *zip;
            auto zip_files = frame::list_zip_files(*zip);
            for (auto& z : zip_files)
            {
                //auto data = frame::get_zip_file(*zip, z);
                result.available_models.insert(std::move(z));
            }
            return result;
        };

        for (size_t i = 0; i < files.size(); i++)
            *zips[i] = create_model_zip(files_data[files[i]]);
    });
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

float get_rotation_angle(double jd0, double P, double jd)
{
    return frame::deg_to_rad(float((jd - jd0) * 360.0 / P));
}

HMM_Mat4 create_rotation(const body_node& node)
{
    if (node.rotation_period == 0.0)
        return HMM_M4D(1.0f);

    auto angle = get_rotation_angle(node.rotation_jd0, node.rotation_period, commons::INIT_TIME_JULIAN + state.time_offset);

    return HMM_Rotate_RH(angle, HMM_Vec3{ node.rotation_axis.x, node.rotation_axis.y, node.rotation_axis.z });
}

HMM_Mat4 create_random_rotation()
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

std::pair<HMM_Mat4, HMM_Mat4> create_world_mvp(const body_node& body, float radius)
{
    static const float mesh_size = 2.0f;

    float scale_factor = radius * 2.0f / mesh_size;

    auto scale = HMM_Scale({ scale_factor, scale_factor, scale_factor });
    HMM_Mat4 rotate;
    if (settings.model_rotation_random)
        rotate = create_random_rotation();
    else
        rotate = create_rotation(body);
    auto translate = HMM_Translate({ body.current_position.x, body.current_position.y, body.current_position.z });
    HMM_Mat4 model_matrix = HMM_MulM4(HMM_MulM4(translate, rotate), scale);

    return { create_projection_view_matrix(), model_matrix };
}

body_draw_model_vs_params_t create_vs_params(const body_node& body, float radius)
{
    body_draw_model_vs_params_t result = {};

    std::tie(result.projection_view, result.model) = create_world_mvp(body, radius);

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

    auto body_position = body.get_absolute_position();

    body_draw_model_fs_params_t params_fs = create_fs_params(color, body_position);
    body_draw_model_vs_params_t params_vs = create_vs_params(body, radius);

    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_body_draw_model_vs_params, SG_RANGE(params_vs));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_body_draw_model_fs_params, SG_RANGE(params_fs));

    sg_draw(0, element_count, 1);

    if (body.rotation_period)
    {
        auto scale = HMM_Scale({ radius * 0.02f, radius * 5.0f, radius * 0.02f });
        auto model = HMM_MulM4(create_hmm_direction(body.rotation_axis), scale);
        auto mvp = HMM_MulM4(create_projection_view_matrix(), model);
        frame::draw_cylinder(mvp, col4::RGBf(0.5f, 0.5f, 0.5f), sshapes_shading::flat, -vec3(body.get_absolute_position()), model);
    }

    return true;
}
