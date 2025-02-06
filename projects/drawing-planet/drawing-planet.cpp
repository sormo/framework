#include <framework.h>
#include <vector>
#include <string>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include <sokol_shape.h>
#include <stb_image.h>
#include "imgui.h"
#include "utils.h"
#include "drawing_sg.h"
#include "svg.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>
#undef HANDMADE_MATH_IMPLEMENTATION

#include "planet.glsl.h"

using namespace frame;

frame::free_move_camera_config free_move_config;

static const float planet_radius = 250.0f;
static const float axis_length = 600.0f;
static const float axis_width = 5.0f;

struct 
{
    struct
    {
        sg_pipeline pip;
        sg_bindings bind;

    } planet;

    float light_position[3] = { 0.0f, 0.0f, 1.0f };
    float light_distance = 6.0f;

    float light_power = 40.0f;
    float ambient_color[3] = { 0.01f, 0.01f, 0.01f };
    float diffuse_color[3] = { 0.75f, 0.75f, 0.75f };
    float specular_color[3] = { 0.1f, 0.1f, 0.1f };
    float shininess = 16.0f;

    float atmosphere_radius_relative = 0.25f;
    float atmosphere_color[3] = { 1.0f, 0.9f, 0.7f };
    float atmosphere_density = 1.0f;

    // ---

    float valuex = 1.0f;
    float valuey = 1.0f;
    float valuez = 1.0f;

} state;

void update_sshape1()
{
    auto direction = vec3(state.valuex, state.valuey, state.valuez).normalized();
    auto orientation = frame::mat4::look_at(direction);
    auto model = mat4::translation(-direction * axis_length / 4.0f) * orientation * mat4::scaling({ axis_width, axis_length / 2.0f, axis_width });
    //auto model = HMM_MulM4(HMM_Translate(to_hmm(-direction * axis_length / 4.0f)), HMM_MulM4(orientation, HMM_Scale({ axis_width, axis_length / 2.0f, axis_width })));
    auto mvp = create_world_projection_view() * model;
    frame::draw_cylinder(mvp, col4::RGBf(0.8f, 0.8f, 0.8f), sshapes_shading::flat, { state.light_position[0], state.light_position[1], state.light_position[2] }, model);
}

void update_sshape2()
{
    auto real_planet_radius = planet_radius * 0.5f;

    //frame::draw_sphere(create_world_mvp(vec3{}, 0.0f, { real_planet_radius * 2.0f }), col4::BLANK);

    auto direction = vec3(state.valuex, state.valuey, state.valuez).normalized();
    auto length = axis_length/2.0f - real_planet_radius;

    auto orientation = frame::mat4::look_at(direction);
    auto model = mat4::translation(direction * (axis_length / 2.0f + real_planet_radius) / 2.0f) * orientation * mat4::scaling({ axis_width, length, axis_width });
    //auto model = HMM_MulM4(HMM_Translate(to_hmm(direction * (axis_length/2.0f + real_planet_radius) / 2.0f)), HMM_MulM4(orientation, HMM_Scale({ axis_width, length, axis_width })));
    auto mvp = create_world_projection_view() * model;
    frame::draw_cylinder(mvp, col4::RGBf(0.8f, 0.8f, 0.8f), sshapes_shading::flat, {state.light_position[0], state.light_position[1], state.light_position[2]}, model);
}

void setup_planet()
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
    state.planet.bind.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    uint16_t indices[] = { 0, 1, 3, 2 };
    sg_buffer_desc buffer_desc_index = {};
    buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
    buffer_desc_index.data = SG_RANGE(indices);
    buffer_desc_index.label = "planet-indices";
    state.planet.bind.index_buffer = sg_make_buffer(&buffer_desc_index);

    sg_shader shd = sg_make_shader(planet_shader_desc(sg_query_backend()));

    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    pipeline_desc.layout.attrs[ATTR_planet_position].format = SG_VERTEXFORMAT_FLOAT2;
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

    state.planet.pip = sg_make_pipeline(&pipeline_desc);
}

fs_params_planet_t create_planet_fs_params()
{
    auto to_vec3 = [](const float* d) { return vec3(d[0], d[1], d[2]); };

    fs_params_planet_t result = {};
    result.light_power = state.light_power;
    result.ambient_color = to_vec3(state.ambient_color);
    result.diffuse_color = to_vec3(state.diffuse_color);
    result.specular_color = to_vec3(state.specular_color);
    result.shininess = state.shininess;

    // for phong and blinn-phong we need vector toward the camera, problem is that fragment shader uses [-0.5, 0.5] range in the shader
    // the whole planet is scaled up
    //auto camera_center = get_world_rectangle().center() / (2.0f * 500.0f);
    //float camera_position[3] = { camera_center.x, camera_center.y,  1.0f / get_world_scale().x };
    result.camera_position = vec3{ 0.0f, 0.0f, 1.0f };

    // compute light position
    result.light_position = vec3(state.light_position[0], state.light_position[1], state.light_position[2]).normalized() * state.light_distance;

    result.atmosphere_radius_relative = state.atmosphere_radius_relative;
    result.atmosphere_color = to_vec3(state.atmosphere_color);
    result.atmosphere_density = state.atmosphere_density;

    return result;
}

static vs_params_planet_t create_planet_vs_params()
{
    vs_params_planet_t result = {};
    result.mvp = frame::create_world_mvp(vec2{}, 0.0f, { planet_radius * 2.0f });

    return result;
}

void update_planet()
{
    sg_apply_pipeline(state.planet.pip);
    sg_apply_bindings(&state.planet.bind);

    auto params_vs = create_planet_vs_params();
    sg_apply_uniforms(UB_vs_params_planet, SG_RANGE(params_vs));

    auto params_fs = create_planet_fs_params();
    sg_apply_uniforms(UB_fs_params_planet, SG_RANGE(params_fs));

    sg_draw(0, 4, 1);
}

static vec3 get_rotation_axis(double lambda, double beta)
{
    vec3 rotation_axis;
    rotation_axis.x = cosf(beta) * cosf(lambda);
    rotation_axis.y = cosf(beta) * sinf(lambda);
    rotation_axis.z = sinf(beta);
    return rotation_axis;
}

void setup()
{
    frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, -1.0f }));
    free_move_config.min_size = { 0.1f, 0.1f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 1'000'000.0f, 1'000'000.0f });

    setup_planet();

    // set the angle
    auto axis = get_rotation_axis(frame::deg_to_rad(339.867), frame::deg_to_rad(84.960));
    state.valuex = axis.x;
    state.valuey = axis.y;
    state.valuez = axis.z;
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

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Scale");
    ImGui::Text("%.2f %.2f ", frame::get_world_scale().x, frame::get_world_scale().y);

    ImGui::EndMainMenuBar();

    bool open = true;
    ImGui::SetNextWindowPos({ 0.0f, 20.0f });
    ImGui::SetNextWindowSize({});
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::DragFloat3("Light Position", state.light_position, 0.01f, -1.0f, 1.0f);
    ImGui::ColorEdit3("Diffuse Color", state.diffuse_color);
    ImGui::DragFloat("Atmosphere Radius", &state.atmosphere_radius_relative, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Atmosphere Density", &state.atmosphere_density, 0.01f, 0.0f, 20.0f);

    if (ImGui::CollapsingHeader("Advanced"))
    {
        ImGui::InputFloat("Light Distance", &state.light_distance);
        ImGui::InputFloat("Light Power", &state.light_power);
        ImGui::ColorEdit3("Ambient Color", state.ambient_color);
        ImGui::ColorEdit3("Specular Color", state.specular_color);
        ImGui::InputFloat("Shininess", &state.shininess);
        ImGui::ColorEdit3("Atmosphere Color", state.atmosphere_color);
    }

    // ---

    ImGui::DragFloat("valuex", &state.valuex, 0.01f, -1.0f, 1.0f);
    ImGui::DragFloat("valuey", &state.valuey, 0.01f, -1.0f, 1.0f);
    ImGui::DragFloat("valuez", &state.valuez, 0.01f, -1.0f, 1.0f);

    ImGui::End();

    //ImGui::ShowDemoWindow();
}

void update()
{
    frame::begin_default_pass();

    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    update_imgui();

    update_sshape1();
    update_planet();
    update_sshape2();
    frame::draw_gizmo(create_world_projection_view(), 200.0f, 5.0f);

    frame::free_move_camera_update(free_move_config);

    frame::end_pass();
}
