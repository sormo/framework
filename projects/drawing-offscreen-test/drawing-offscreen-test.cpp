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

#include "offscreen.glsl.h"

using namespace frame;

frame::free_move_camera_config free_move_config;

struct
{
    image_t render_target;

} state;

void setup_offscreen()
{
    state.render_target = frame::create_image_target(256, 256, { SG_PIXELFORMAT_RGBA8, 1, true });
}

void update_offscreen_non_default_pass()
{
    auto projection = mat4::orthographic(-128.0f, 128.0f, -128.0f, 128.0f, -max_depth, max_depth);
    auto model = mat4::scaling(100.0f);
    frame::draw_sphere(projection * model, col4::ORANGE, sshapes_shading::flat, { 5.0f, 5.0f, 10.0f }, model);
}

void update_offscreen()
{
    frame::draw_image_ex(state.render_target, {}, .0f, { 500.0f, 500.0f }, text_align::middle_middle);
}

void setup()
{
    frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, -1.0f }));
    free_move_config.min_size = { 0.1f, 0.1f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 1'000'000.0f, 1'000'000.0f });

    setup_offscreen();
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

    ImGui::End();

    //ImGui::ShowDemoWindow();
}

void update()
{
    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    update_imgui();

    begin_pass_clear(state.render_target, col4::DARKGRAY);
    update_offscreen_non_default_pass();
    end_pass();

    begin_default_pass();
    update_offscreen();
    end_pass();

    frame::free_move_camera_update(free_move_config);
}
