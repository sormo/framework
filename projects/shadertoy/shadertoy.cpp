#include <framework.h>
#include <vector>
#include <string>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include <stb_image.h>
#include <chrono>
#include "imgui.h"
#include "utils.h"
#include "drawing_sg.h"
#include "svg.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>
#undef HANDMADE_MATH_IMPLEMENTATION

#include "shadertoy.glsl.h"

using namespace frame;

struct 
{
    sg_pipeline pip_shadertoy;
    sg_bindings bind_shadertoy;

    std::chrono::system_clock::time_point start_app;
    std::chrono::system_clock::time_point start_frame;
    int32_t frame_count = 0;
    frame::vec2 last_mouse_press;

    float frame_delta;

} state;

void setup_shadertoy()
{
    struct vertex_t
    {
        float x, y;
    };

    vertex_t vertices[] =
    {
         1.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         0.0f, 1.0f
    };

    sg_buffer_desc buffer_desc_vert = {};
    buffer_desc_vert.data = SG_RANGE(vertices);
    buffer_desc_vert.label = "shadertoy-vertices";
    state.bind_shadertoy.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    uint16_t indices[] = { 0, 1, 3, 2 };
    sg_buffer_desc buffer_desc_index = {};
    buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
    buffer_desc_index.data = SG_RANGE(indices);
    buffer_desc_index.label = "shadertoy-indices";
    state.bind_shadertoy.index_buffer = sg_make_buffer(&buffer_desc_index);

    sg_shader shd = sg_make_shader(shadertoy_shader_desc(sg_query_backend()));

    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    pipeline_desc.layout.attrs[ATTR_shadertoy_uv].format = SG_VERTEXFORMAT_FLOAT2;
    pipeline_desc.shader = shd;
    pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    pipeline_desc.label = "shadertoy-pipeline";
    state.pip_shadertoy = sg_make_pipeline(&pipeline_desc);
}

float get_seconds_since_app_start()
{
    auto duration = std::chrono::system_clock::now() - state.start_app;

    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

void fill_date(float date[4])
{
    auto time = std::time(0);
    std::tm* now = std::localtime(&time);

    date[0] = now->tm_year - 1;
    date[1] = now->tm_mon;
    date[2] = now->tm_mday;
    date[3] = now->tm_sec; // TODO fraction of second
}

void update_shadertoy()
{
    sg_apply_pipeline(state.pip_shadertoy);
    sg_apply_bindings(&state.bind_shadertoy);

    vs_params_shadertoy_t vs_params;
    //vs_params.mvp = create_world_projection_view() * mat4::transform({}, 0.0f, get_screen_size());
    vs_params.mvp = mat4::orthographic(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 100.0f);

    sg_apply_uniforms(UB_vs_params_shadertoy, SG_RANGE(vs_params));

    fs_params_shadertoy_t fs_params;
    fs_params.iResolution[0] = sapp_widthf();
    fs_params.iResolution[1] = sapp_heightf();
    fs_params.iTime = get_seconds_since_app_start();
    fill_date(fs_params.iDate);
    fs_params.iFrame = state.frame_count;
    fs_params.iTimeDelta = state.frame_delta;
    fs_params.iMouse[0] = state.last_mouse_press.x;
    fs_params.iMouse[1] = state.last_mouse_press.y;

    sg_apply_uniforms(UB_fs_params_shadertoy, SG_RANGE(fs_params));

    sg_draw(0, 4, 1);
}

void setup()
{
    //frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, -1.0f }));
    //frame::set_world_transform(frame::scale({ 1.0f, -1.0f }));

    setup_shadertoy();

    state.start_app = std::chrono::system_clock::now();
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

    ImGui::End();
}

void update()
{
    frame::begin_default_pass();

    state.frame_count++;

    auto now = std::chrono::system_clock::now();
    state.frame_delta = std::chrono::duration_cast<std::chrono::seconds>(now - state.start_frame).count();
    state.start_frame = now;

    if (frame::is_mouse_down(frame::mouse_button::left))
        state.last_mouse_press = frame::get_mouse_screen_position();

    update_imgui();
    update_shadertoy();

    frame::end_pass();
}
