#include <framework.h>
#include <vector>
#include <string>
#include <sstream>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include <stb_image.h>
#include "imgui.h"
#include "tiny_obj_loader.h"
#include "utils.h"
#include "drawing_sg.h"
#include "svg.h"
#include "HandmadeMath.h"
#include "model3d.glsl.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>
#undef HANDMADE_MATH_IMPLEMENTATION

using namespace frame;

static const float MeshSize = 2.0f; // size of the mesh in model or object space
static const float MeshScreenSize = 200.0f; // desired size of mesh in screen space

frame::free_move_camera_config free_move_config;

struct draw_state_t
{
    sg_pipeline pip = {};
    sg_bindings bind = {};
    size_t count = 0;
};

enum draw_type_t : int
{
    flat = 0,
    smooth
};

enum projection_t : int
{
    orthogonal = 0,
    perspective
};

struct box_t
{
    vec3 min;
    vec3 max;
};

struct
{
    draw_state_t smooth;
    draw_state_t flat;

    float znear = 3.0f;
    float zfar = 100.0f;
    float view_zpos = -5.0f;
    float rotation = 0.01f;
    draw_type_t draw_type = draw_type_t::flat;
    projection_t projection_type = projection_t::perspective;

    float light_position[3] = { 0.0f, 0.0f, 200.0f };
    float light_color[3] = { 1.0f, 0.75f, 0.5f };
    float object_color[3] = { 1.0f, 1.0f, 1.0f };
    float ambient_strength = 0.001f;
    float specular_strength = 0.1f;

    model_t flat_model;
    model_t smooth_model;

} state;

box_t compute_bounding_box(const std::vector<vertex_t>& vertices)
{
    box_t result;
    result.max.x = result.max.y = result.max.z = std::numeric_limits<float>::min();
    result.min.x = result.min.y = result.min.z = std::numeric_limits<float>::max();

    for (const auto& t : vertices)
    {
        result.max.x = std::max(t.position.x, result.max.x);
        result.max.y = std::max(t.position.y, result.max.y);
        result.max.z = std::max(t.position.z, result.max.z);
        result.min.x = std::min(t.position.x, result.min.x);
        result.min.y = std::min(t.position.y, result.min.y);
        result.min.z = std::min(t.position.z, result.min.z);
    }

    return result;
}

vec2 convert_to_screen(const HMM_Mat4& mvp, const vec3& point_model_space)
{
    auto clip_space = HMM_MulM4V4(mvp, HMM_Vec4{ point_model_space.x, point_model_space.y, point_model_space.z, 1.0f });

    auto perspective_division = [](HMM_Vec4 v)
    {
        if (v.W == 0.0f)
            return v;
        v.W = 1.0f / v.W;
        v.X *= v.W;
        v.Y *= v.W;
        v.Z *= v.W;
        return v;
    };

    auto ndc = perspective_division(clip_space);

    vec2 result;
    result.x = (ndc.X + 1.0f) * sapp_widthf() / 2.0f;
    result.y = (1.0f - ndc.Y) * sapp_heightf() / 2.0f;

    return result;
}

rectangle get_model_size_on_screen(const HMM_Mat4& mvp, const model_t& model)
{
    std::vector<vec2> converted_vertices(model.vertices.size());
    for (size_t i = 0; i < model.vertices.size(); i++)
        converted_vertices[i] = convert_to_screen(mvp, model.vertices[i].position);

    // NDC to screen space
    rectangle result;
    result.max.x = result.max.y = std::numeric_limits<float>::min();
    result.min.x = result.min.y = std::numeric_limits<float>::max();

    for (const auto& t : converted_vertices)
    {
        result.max.x = std::max(t.x, result.max.x);
        result.max.y = std::max(t.y, result.max.y);
        result.min.x = std::min(t.x, result.min.x);
        result.min.y = std::min(t.y, result.min.y);
    }

    return result;
}
std::optional<model_t> load_smooth(const std::vector<char>& data)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::istringstream obj_stream(data.data());

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &obj_stream, nullptr, true);

    if (!ret)
        return {};

    std::unordered_map<tinyobj::index_t, uint16_t, tinyobj::index_t_hasher> index_map;
    model_t result;

    for (size_t i = 2; i < attrib.vertices.size(); i += 3)
        result.vertices.push_back({ vec3(attrib.vertices[i - 2], attrib.vertices[i - 1], attrib.vertices[i - 0]) });

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            result.indices.push_back(index.vertex_index);
            // TODO this is possibly not good, do we need separate buffer for normals ?
            if (index.normal_index != -1)
                result.vertices[index.vertex_index].normal = attrib.normals[index.normal_index];
        }
    }

    auto merge_normals = [](const vec3& n1, const vec3 n2) -> vec3
    {
        static const float dot_threshold = std::cos(frame::deg_to_rad(30.0f));

        if (n1.dot(n2) > dot_threshold)
            return n2;
        return n1 + n2;
    };

    // compute missing normals
    for (size_t i = 2; i < result.indices.size(); i += 3)
    {
        const vec3& v0 = result.vertices[result.indices[i - 0]].position;
        const vec3& v1 = result.vertices[result.indices[i - 1]].position;
        const vec3& v2 = result.vertices[result.indices[i - 2]].position;

        vec3 normal = (v1 - v0).cross(v2 - v0);

        result.vertices[result.indices[i - 2]].normal = merge_normals(result.vertices[result.indices[i - 2]].normal, normal);
        result.vertices[result.indices[i - 1]].normal = merge_normals(result.vertices[result.indices[i - 1]].normal, normal);
        result.vertices[result.indices[i - 0]].normal = merge_normals(result.vertices[result.indices[i - 0]].normal, normal);
    }

    // normalize normals
    for (auto& v : result.vertices)
        v.normal.normalize();

    return result;
}

void init_smooth(const std::vector<char>& data, draw_state_t& draw_state)
{
    auto model = load_smooth(data);

    sg_buffer_desc buffer_desc_vert = {};
    buffer_desc_vert.data = sg_range{ (void*)model->vertices.data(), model->vertices.size() * sizeof(vertex_t) };
    buffer_desc_vert.label = "model3d-vertices-smooth";
    draw_state.bind.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    sg_buffer_desc buffer_desc_index = {};
    buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
    buffer_desc_index.data = sg_range{ (void*)model->indices.data(), model->indices.size() * sizeof(uint16_t) };
    buffer_desc_index.label = "model3d-indices-smooth";
    draw_state.bind.index_buffer = sg_make_buffer(&buffer_desc_index);

    draw_state.count = model->indices.size();

    sg_pipeline_desc pip_desc = {};
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    pip_desc.shader = sg_make_shader(model3d_shader_desc(sg_query_backend()));;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;

    pip_desc.layout.attrs[ATTR_model3d_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_model3d_vs_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

    draw_state.pip = sg_make_pipeline(pip_desc);

    state.smooth_model = *model;
}

void init_flat(const std::vector<char>& data, draw_state_t& draw_state)
{
    // loading flat is already in utils
    auto model = load_obj_flat(data);

    sg_buffer_desc buffer_desc_vert = {};
    buffer_desc_vert.data = sg_range{ (void*)model->vertices.data(), model->vertices.size() * sizeof(vertex_t) };
    buffer_desc_vert.label = "model3d-vertices-flat";
    draw_state.bind.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    draw_state.count = model->vertices.size();

    sg_pipeline_desc pip_desc = {};
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    pip_desc.shader = sg_make_shader(model3d_shader_desc(sg_query_backend()));;

    pip_desc.layout.attrs[ATTR_model3d_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[ATTR_model3d_vs_normal].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.depth.write_enabled = true;
    pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

    draw_state.pip = sg_make_pipeline(pip_desc);

    state.flat_model = *model;
}

HMM_Mat4 create_perspective_projection()
{
    const float w = sapp_widthf();
    const float h = sapp_heightf();
    return HMM_Perspective_RH_NO(frame::deg_to_rad(45.0f), w / h, state.znear, state.zfar);
}

HMM_Mat4 create_perspective_view()
{
    //return HMM_LookAt_RH(HMM_Vec3{ 0.0f, 0.0f, 5.0f }, HMM_Vec3{ 0.0f, 0.0f, 0.0f }, HMM_Vec3{ 0.0f, 1.0f, 0.0f });

    HMM_Mat4 result = create_hmm_transform(frame::get_world_transform());
    // scale also z-coordinate, why not needed in orthogonal ?
    result.Elements[2][2] = result.Elements[0][0];
    // set distance in z-coordinate
    //result.Columns[3][2] = -5.0f;

    return result;
}

HMM_Mat4 create_rotation()
{
    static float rx = 0.0f;
    static float ry = 0.0f;

    HMM_Mat4 rxm = HMM_Rotate_RH(rx, HMM_Vec3{ 1.0f, 0.0f, 0.0f });
    HMM_Mat4 rym = HMM_Rotate_RH(ry, HMM_Vec3{ 0.0f, 1.0f, 0.0f });

    const float t = (float)(sapp_frame_duration() * 60.0);
    rx += state.rotation * t; ry += (state.rotation/2.0f) * t;

    return HMM_MulM4(rxm, rym);
}

float compute_perspective_scale_factor(float screen_size)
{
    //return screen_size / (2.0f * std::tan(frame::deg_to_rad(45.0f) / 2.0f) * 5.0f);

    float d = 7.177f; // TODO distance on z coordinate when ndcZ is set to 0.2f
    float w_desired = screen_size;
    float w_model = MeshSize;
    float f = sapp_widthf() / (2.0f * std::tan(frame::deg_to_rad(45.0f) / 2.0f));

    return (w_desired * d) / (w_model * f);
}

vec3 compute_perspective_translation(const HMM_Mat4& projectionView, const vec2& screen_position)
{
    auto screen = get_world_transform().transform_point(screen_position);

    float ndcX = (2.0f * screen.x / sapp_widthf()) - 1.0f;
    float ndcY = 1.0f - (2.0f * screen.y / sapp_height());
    float ndcZ = 0.2f; // TODO why ? if I move camera from 0.0f model starts to move away
    //float ndcZ = 2.0f * 0.0f - 1.0f; // https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluUnProject.xml

    HMM_Vec4 ndcPosition = HMM_Vec4{ ndcX, ndcY, ndcZ, 1.0f };
    HMM_Mat4 viewProjectionInvMatrix = HMM_InvGeneralM4(projectionView);
    HMM_Vec4 worldPosition = HMM_MulM4V4(viewProjectionInvMatrix, ndcPosition);

    // Perform perspective divide
    if (worldPosition.W != 0.0f) {
        worldPosition.W = 1.0f / worldPosition.W;
        worldPosition.X *= worldPosition.W;
        worldPosition.Y *= worldPosition.W;
        worldPosition.Z *= worldPosition.W;
    }
    //worldPosition.Z = 0.0f;

    return { worldPosition.X, worldPosition.Y, worldPosition.Z };
}

std::pair<HMM_Mat4, HMM_Mat4> create_perspective_mvp()
{
    HMM_Mat4 viewMatrix = create_perspective_view();
    HMM_Mat4 projectionMatrix = create_perspective_projection();

    auto worldTranslation = compute_perspective_translation(HMM_MulM4(projectionMatrix, viewMatrix), {});
    HMM_Mat4 modelTranslation = HMM_Translate(HMM_Vec3{ worldTranslation.x, worldTranslation.y, worldTranslation.z });
    HMM_Mat4 modelRotation = create_rotation();
    auto scale = compute_perspective_scale_factor(MeshScreenSize);
    HMM_Mat4 modelScale = HMM_Scale({ scale, scale, scale });

    HMM_Mat4 modelMatrix = HMM_MulM4(modelTranslation, HMM_MulM4(modelRotation, modelScale));

    return { HMM_MulM4(projectionMatrix, viewMatrix), modelMatrix };
}

std::pair<HMM_Mat4, HMM_Mat4> create_orthogonal_mvp()
{
    float scale_factor = MeshScreenSize / MeshSize;

    HMM_Mat4 modelMatrix = HMM_Scale(HMM_Vec3{ scale_factor, scale_factor, scale_factor });
    modelMatrix = HMM_MulM4(modelMatrix, create_rotation());

    return { create_projection_view_matrix(), modelMatrix };
}

model3d_vs_params_t create_vs_params()
{
    model3d_vs_params_t result;
    if (state.projection_type == projection_t::perspective)
        std::tie(result.projection_view, result.model) = create_perspective_mvp();
    else
        std::tie(result.projection_view, result.model) = create_orthogonal_mvp();
    return result;
}

model3d_fs_params_t create_fs_params()
{
    auto to_vec3 = [](const float* d) { return vec3(d[0], d[1], d[2]); };

    model3d_fs_params_t result;

    result.light_color = to_vec3(state.light_color);
    result.object_color = to_vec3(state.object_color);
    result.ambient_strength = state.ambient_strength;
    result.specular_strength = state.specular_strength;

    result.view_position = vec3(0.0f, 0.0f, state.view_zpos);
    if (state.projection_type == projection_t::perspective)
        result.light_position = vec3(-state.light_position[0], -state.light_position[1], state.light_position[2]); // TODO light position for perspective does not work at all
    else
        result.light_position = vec3(state.light_position[0], state.light_position[1], state.light_position[2]);

    return result;
}

void draw(const draw_state_t& draw_state)
{
    if (draw_state.pip.id == 0)
        return;

    model3d_vs_params_t vs_params = create_vs_params();
    model3d_fs_params_t fs_params = create_fs_params();

    sg_apply_pipeline(draw_state.pip);
    sg_apply_bindings(&draw_state.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_model3d_vs_params, SG_RANGE(vs_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_model3d_fs_params, SG_RANGE(fs_params));
    sg_draw(0, draw_state.count, 1);
}

void setup_sg()
{
    frame::fetch_file("Astraea.obj", [](std::vector<char> data)
    {
        init_smooth(data, state.smooth);
        init_flat(data, state.flat);
    });
}

void draw_sg()
{
    if (state.draw_type == draw_type_t::smooth)
        draw(state.smooth);
    else
        draw(state.flat);
}

void setup()
{
    setup_sg();

    frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, -1.0f }));
    free_move_config.min_size = { 0.1f, 0.1f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 1'000'000.0f, 1'000'000.0f });
}

void update_imgui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_canvas = frame::get_screen_to_world(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", frame::get_screen_size().x, frame::get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", frame::get_world_size().x, frame::get_world_size().y);

    ImGui::EndMainMenuBar();

    bool open = true;
    ImGui::SetNextWindowPos({ 0.0f, 20.0f });
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::SliderFloat("znear", &state.znear, -1.0f, 10.0f);
    ImGui::SliderFloat("zfar", &state.zfar, -1.0f, 100.0f);
    ImGui::SliderFloat("View zpos", &state.view_zpos, -300.0f, 300.0f);
    ImGui::SliderFloat("rotation", &state.rotation, 0.05f, 0.0001f);

    ImGui::Combo("Draw type", (int*)&state.draw_type, "flat\0smooth");
    ImGui::Combo("Projection", (int*)&state.projection_type, "orthogonal\0perspective");

    if (ImGui::CollapsingHeader("Light"))
    {
        ImGui::SliderFloat3("Light position", state.light_position, -200.0f, 200.0f);
        ImGui::ColorEdit3("Light color", state.light_color);
        ImGui::ColorEdit3("Object color", state.object_color);
        ImGui::InputFloat("Ambient strength", &state.ambient_strength);
        ImGui::InputFloat("Specular strength", &state.specular_strength);
    }

    ImGui::End();
}

void update()
{
    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    draw_sg();

    if (state.projection_type == projection_t::perspective)
    {
        auto [projection_view, model] = create_perspective_mvp();
        auto rectangle = get_model_size_on_screen(HMM_MulM4(projection_view, model), state.flat_model);

        rectangle.min = frame::get_screen_to_world(rectangle.min);
        rectangle.max = frame::get_screen_to_world(rectangle.max);

        //draw_rectangle(rectangle, frame::col4::RGBf(1.0f, 0.5f, 0.25f, 0.5f));
    }

    draw_circle({ state.light_position[0], state.light_position[1]}, 3.0f, col4::WHITE);

    update_imgui();

    frame::free_move_camera_update(free_move_config);
}
