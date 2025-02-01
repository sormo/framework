#if defined(__EMSCRIPTEN__) || defined(__ANDROID_API__)
#define SOKOL_GLES3
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <Windows.h>
#define SOKOL_IMPL
#define SOKOL_D3D11
#endif
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "glm/glm.hpp"
#include "triangle.glsl.h"

static struct
{
    sg_pipeline pip;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;

static void init(void)
{
    sg_desc setup_desc = {};
    setup_desc.environment = sglue_environment();
    setup_desc.logger.func = slog_func;

    sg_setup(&setup_desc);

    float vertices[] =
    {
         0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f
    };

    sg_buffer_desc buffer_desc = {};
    buffer_desc.data = SG_RANGE(vertices);

    state.bind.vertex_buffers[0] = sg_make_buffer(buffer_desc);

    sg_pipeline_desc pipeline = {};
    pipeline.shader = sg_make_shader(triangle_shader_desc(sg_query_backend()));
    pipeline.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
    pipeline.layout.attrs[ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4;

    state.pip = sg_make_pipeline(pipeline);

    state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    state.pass_action.colors[0].clear_value = { 0.1f, 0.3f, 5.0f, 1.0f };
}

void frame(void)
{
    sg_pass pass = {};
    pass.action = state.pass_action;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(pass);
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_draw(0, 3, 1);
    sg_end_pass();
    sg_commit();
}

void cleanup(void)
{
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc; (void)argv;

    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.width = 640;
    desc.height = 480;
    desc.window_title = "Triangle";
    desc.icon.sokol_default = true;
    desc.logger.func = slog_func;

    return desc;
}
