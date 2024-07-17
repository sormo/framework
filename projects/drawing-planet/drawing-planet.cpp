#include <framework.h>
#include <vector>
#include <string>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include <stb_image.h>
#include "imgui.h"
#include "utils.h"
#include "drawing_sg.h"
#include "svg.h"
#include <HandmadeMath.h>
#include "planet.glsl.h"

using namespace frame;

frame::free_move_camera_config free_move_config;

struct 
{
    sg_pipeline pip_planet;
    sg_bindings bind_planet;

    float planet_color[3] = { 1.0f, 1.0f, 1.0f };
    float light_position[3] = { 0.0f, 0.0f, 1.0f };
    float light_distance = 10.0f;

    float light_power = 40.0f;
    float ambient_color[3] = { 0.1f, 0.1f, 0.1f };
    float diffuse_color[3] = { 0.1f, 0.1f, 0.1f };
    float spec_color[3] = { 0.1f, 0.1f, 0.1f };
    float shininess = 16.0f;
    float screen_gamma = 2.0f;
    int light_mode = 0;

} state;

void setup_planet()
{
    struct vertex_t
    {
        float x, y;
    };

    vertex_t vertices[] =
    {
         0.5f,  0.5f, 1.0f, 1.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };

    sg_buffer_desc buffer_desc_vert = {};
    buffer_desc_vert.data = SG_RANGE(vertices);
    buffer_desc_vert.label = "planet-vertices";
    state.bind_planet.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    uint16_t indices[] = { 0, 1, 3, 2 };
    sg_buffer_desc buffer_desc_index = {};
    buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
    buffer_desc_index.data = SG_RANGE(indices);
    buffer_desc_index.label = "planet-indices";
    state.bind_planet.index_buffer = sg_make_buffer(&buffer_desc_index);

    sg_shader shd = sg_make_shader(planet_shader_desc(sg_query_backend()));

    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    pipeline_desc.layout.attrs[ATTR_planet_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
    pipeline_desc.layout.attrs[ATTR_planet_vs_uv].format = SG_VERTEXFORMAT_FLOAT2;
    pipeline_desc.shader = shd;
    pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    pipeline_desc.label = "planet-pipeline";
    state.pip_planet = sg_make_pipeline(&pipeline_desc);
}

hmm_mat4 create_hmm_transform(const frame::mat3& transform)
{
    hmm_mat4 result = HMM_Mat4d(1.0f);

    result.Elements[0][0] = transform.data[0];
    result.Elements[0][1] = transform.data[3];
    result.Elements[0][3] = transform.data[6];

    result.Elements[1][0] = transform.data[1];
    result.Elements[1][1] = transform.data[4];
    result.Elements[1][3] = transform.data[7];

    result.Elements[3][0] = transform.data[2];
    result.Elements[3][1] = transform.data[5];
    result.Elements[3][3] = transform.data[8];

    return result;
}

hmm_mat4 create_projection_view_matrix()
{
    hmm_mat4 view = create_hmm_transform(frame::get_world_transform());
    hmm_mat4 projection = HMM_Orthographic(0.0f, sapp_widthf(), sapp_heightf(), 0.0f, 0.0f, 100.0f);

    return HMM_MultiplyMat4(projection, view);
}

hmm_mat4 create_hmm_transform(frame::vec2 position, float rotation, frame::vec2 size)
{
    auto scale = HMM_Scale({ size.x, size.y, 0.0f });
    auto rotate = HMM_Rotate(rotation, HMM_Vec3(0.0f, 0.0f, 1.0f));
    auto translate = HMM_Translate({ position.x, position.y, 0.0f });
    return HMM_MultiplyMat4(HMM_MultiplyMat4(translate, rotate), scale);
}

void update_planet()
{
    sg_apply_pipeline(state.pip_planet);
    sg_apply_bindings(&state.bind_planet);

    auto assign = [](const float* from, float* to, size_t count) { for (size_t i = 0; i < count; i++) to[i] = from[i]; };

    // compute light position, this awkward
    vec3 light_pos_vec(state.light_position[0], state.light_position[1], state.light_position[2]);
    light_pos_vec *= state.light_distance;

    float light_position[3];
    light_position[0] = light_pos_vec.x;
    light_position[1] = light_pos_vec.y;
    light_position[2] = light_pos_vec.z;

    vs_params_planet_t vs_params;
    vs_params.color[0] = vs_params.color[1] = vs_params.color[2] = vs_params.color[3] = 1.0f;
    vs_params.mvp = HMM_MultiplyMat4(create_projection_view_matrix(), create_hmm_transform({}, 0.0f, { 500.0f, 500.0f }));
    assign(light_position, vs_params.light_position, 3);
    assign(state.planet_color, vs_params.color, 3);

    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params_planet, SG_RANGE(vs_params));

    fs_params_planet_t fs_params;
    fs_params.light_power = state.light_power;
    assign(state.ambient_color, fs_params.ambient_color, 3);
    assign(state.diffuse_color, fs_params.diffuse_color, 3);
    assign(state.spec_color, fs_params.spec_color, 3);
    fs_params.shininess = state.shininess;
    fs_params.screen_gamma = state.screen_gamma;
    fs_params.light_mode = state.light_mode;

    // for phong and blinn-phong we need vector toward the camera, problem is that fragment shader uses [-0.5, 0.5] range in the shader
    // the whole planet is scaled up
    auto camera_center = get_world_rectangle().center() / (2.0f * 500.0f);
    float camera_position[3] = { camera_center.x, camera_center.y,  1.0f / get_world_scale().x };
    assign(camera_position, fs_params.camera_position, 3);

    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params_planet, SG_RANGE(fs_params));

    sg_draw(0, 4, 1);
}

void setup()
{
    frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, 1.0f }));
    free_move_config.min_size = { 0.1f, 0.1f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 1'000'000.0f, 1'000'000.0f });

    setup_planet();
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
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::DragFloat3("Light Position", state.light_position, 0.01f, -1.0f, 1.0f);
    ImGui::InputFloat("Light Distance", &state.light_distance);

    ImGui::ColorEdit3("Planet Color", state.planet_color);
    ImGui::InputFloat("Light Power", &state.light_power);
    ImGui::ColorEdit3("Ambient Color", state.ambient_color);
    ImGui::ColorEdit3("Diffuse Color", state.diffuse_color);
    ImGui::ColorEdit3("Spec Color", state.spec_color);
    ImGui::InputFloat("Shininess", &state.shininess);
    ImGui::InputFloat("Screen Gamme", &state.screen_gamma);
    ImGui::Combo("Light Mode", &state.light_mode, "Lambertian\0BlinnPhon\0Phon");

    ImGui::End();
}

void update()
{
    update_imgui();

    update_planet();

    frame::free_move_camera_update(free_move_config);
}
