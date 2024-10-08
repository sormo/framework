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
    struct
    {
        sg_pass pass;
        sg_pipeline pip;
        sg_bindings bind;

        sshape_element_range_t sphere;

    } offscreen;

    struct
    {
        sg_pipeline pip;
        sg_bindings bind;

    } display;

} state;

void setup_offscreen()
{
    static const auto offscreen_pixel_format = SG_PIXELFORMAT_RGBA8;
    static const auto offscreen_sample_count = 1;

    // prepare images for offscreen rendering
    sg_image_desc img_desc = {};

    img_desc.render_target = true;
    img_desc.width = 256;
    img_desc.height = 256;
    img_desc.pixel_format = offscreen_pixel_format;
    img_desc.sample_count = offscreen_sample_count;
    img_desc.label = "color-image";
    sg_image color_img = sg_make_image(&img_desc);

    img_desc.pixel_format = SG_PIXELFORMAT_DEPTH;
    img_desc.label = "depth-image";
    sg_image depth_img = sg_make_image(&img_desc);

    // create offscreen pass
    sg_attachments_desc attachments_desc = {};
    attachments_desc.colors[0].image = color_img;
    attachments_desc.depth_stencil.image = depth_img;
    attachments_desc.label = "offscreen-attachments";

    state.offscreen.pass.attachments = sg_make_attachments(attachments_desc);
    state.offscreen.pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    state.offscreen.pass.action.colors[0].clear_value = { 0.25f, 0.25f, 0.25f, 1.0f };
    state.offscreen.pass.label = "offscreen-pass";

    // create offscreen pipeline
    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.layout.buffers[0] = sshape_vertex_buffer_layout_state();
    pipeline_desc.layout.attrs[ATTR_vs_offscreen_position] = sshape_position_vertex_attr_state();
    pipeline_desc.layout.attrs[ATTR_vs_offscreen_normal] = sshape_normal_vertex_attr_state();
    pipeline_desc.shader = sg_make_shader(offscreen_shader_desc(sg_query_backend()));
    pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    pipeline_desc.cull_mode = SG_CULLMODE_BACK;
    pipeline_desc.sample_count = offscreen_sample_count;
    pipeline_desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH;
    pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
    pipeline_desc.depth.write_enabled = true;
    pipeline_desc.colors[0].pixel_format = offscreen_pixel_format;
    pipeline_desc.label = "offscreen-pipeline";
    state.offscreen.pip = sg_make_pipeline(pipeline_desc);

    // create offscreen bindings
    {
        static sshape_vertex_t vertices[4000] = { 0 };
        static uint16_t indices[24000] = { 0 };
        sshape_buffer_t buf = {};
        buf.vertices.buffer = SSHAPE_RANGE(vertices);
        buf.indices.buffer = SSHAPE_RANGE(indices);
        sshape_sphere_t sphere_desc = {};
        sphere_desc.slices = 72;
        sphere_desc.stacks = 40;
        buf = sshape_build_sphere(&buf, &sphere_desc);
        state.offscreen.sphere = sshape_element_range(&buf);

        sg_buffer_desc vbuf_desc = sshape_vertex_buffer_desc(&buf);
        sg_buffer_desc ibuf_desc = sshape_index_buffer_desc(&buf);
        vbuf_desc.label = "shape-vbuf";
        ibuf_desc.label = "shape-ibuf";
        state.offscreen.bind.vertex_buffers[0] = sg_make_buffer(&vbuf_desc);
        state.offscreen.bind.index_buffer = sg_make_buffer(&ibuf_desc);
    }

    // create display pipeline
    {
        sg_pipeline_desc desc = {};
        desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
        desc.layout.attrs[ATTR_vs_display_position].format = SG_VERTEXFORMAT_FLOAT2;
        desc.shader = sg_make_shader(display_shader_desc(sg_query_backend()));
        desc.index_type = SG_INDEXTYPE_UINT16;
        desc.label = "display-pipeline";
        state.display.pip = sg_make_pipeline(desc);
    }

    // create display bindings (draw quad)
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
        buffer_desc_vert.label = "display-vertices";
        state.display.bind.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

        uint16_t indices[] = { 0, 1, 3, 2 };
        sg_buffer_desc buffer_desc_index = {};
        buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
        buffer_desc_index.data = SG_RANGE(indices);
        buffer_desc_index.label = "display-indices";
        state.display.bind.index_buffer = sg_make_buffer(&buffer_desc_index);

        sg_sampler_desc sampler_desc = {};
        sampler_desc.min_filter = SG_FILTER_LINEAR;
        sampler_desc.mag_filter = SG_FILTER_LINEAR;
        sampler_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        sampler_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        state.display.bind.fs.images[SLOT_tex] = color_img;
        state.display.bind.fs.samplers[SLOT_smp] = sg_make_sampler(sampler_desc);
    }
}

void update_offscreen_non_default_pass()
{
    sg_begin_pass(&state.offscreen.pass);

    sg_apply_pipeline(state.offscreen.pip);
    sg_apply_bindings(&state.offscreen.bind);

    vs_params_t vs_params = {};
    auto projection = mat4::orthographic(-128.0f, 128.0f, -128.0f, 128.0f, -max_depth, max_depth);
    auto model = mat4::scaling(100.0f);
    vs_params.mvp = projection * model;
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));

    sg_draw(state.offscreen.sphere.base_element, state.offscreen.sphere.num_elements, 1);

    sg_end_pass();
}

void update_offscreen()
{
    sg_apply_pipeline(state.display.pip);
    sg_apply_bindings(&state.display.bind);

    vs_params_t vs_params = {};
    vs_params.mvp = frame::create_world_mvp(vec2{}, 0.0f, { 500.0f, 500.0f });
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));

    sg_draw(0, 4, 1);
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

    // TODO this will not work because in update we have already default pass, this function must be called before 
    // sg_begin_pass(pass); in frame_update in framework.cpp
    update_offscreen_non_default_pass();

    update_offscreen();

    frame::free_move_camera_update(free_move_config);
}
