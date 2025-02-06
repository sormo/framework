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
#include "custom_shader.glsl.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>
#undef HANDMADE_MATH_IMPLEMENTATION

using namespace frame;

frame::free_move_camera_config free_move_config;

struct 
{
    sg_pipeline pip;
    sg_bindings bind;

} state;

void setup_sg()
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
    buffer_desc_vert.label = "basic-depth-vertices";
    state.bind.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    uint16_t indices[] = { 0, 1, 3, 2 };
    sg_buffer_desc buffer_desc_index = {};
    buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
    buffer_desc_index.data = SG_RANGE(indices);
    buffer_desc_index.label = "basic-depth-indices";
    state.bind.index_buffer = sg_make_buffer(&buffer_desc_index);

	// pipeline

	sg_pipeline_desc pip_desc = {};
	pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
	pip_desc.shader = sg_make_shader(custom_shader_shader_desc(sg_query_backend()));;
	pip_desc.index_type = SG_INDEXTYPE_UINT16;

	// position attribute in shader (starts at offset 0, it is taken from buffer at index 0 and is two floats)
	pip_desc.layout.attrs[ATTR_custom_shader_position].format = SG_VERTEXFORMAT_FLOAT2;
	pip_desc.layout.attrs[ATTR_custom_shader_position].buffer_index = 0;
	pip_desc.layout.attrs[ATTR_custom_shader_position].offset = 0;
	
	// single buffer with positions at index 0
	pip_desc.layout.buffers[0].stride = 0;
	pip_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
	pip_desc.layout.buffers[0].step_rate = 0;

	state.pip = sg_make_pipeline(pip_desc);
}

void draw_sg()
{
    custom_shader_vs_params_t vs_params;
    vs_params.mvp = frame::create_world_mvp(vec2{}, 0.0f, { 500.0f, 500.0f });

    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(UB_custom_shader_vs_params, SG_RANGE(vs_params));
    sg_draw(0, 4, 1);
}

void setup()
{
    setup_sg();

    frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, 1.0f }));
    free_move_config.min_size = { 0.1f, 0.1f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 1'000'000.0f, 1'000'000.0f });
}

void update()
{
    frame::begin_default_pass();

    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    draw_sg();

    frame::free_move_camera_update(free_move_config);

    frame::end_pass();
}
