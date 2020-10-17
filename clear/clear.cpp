#ifdef __EMSCRIPTEN__
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define SOKOL_IMPL
#include "sokol_gfx.h"
#include "emsc.h"
#else
#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "dbgui/dbgui.h"
#endif

sg_pass_action pass_action;

int app_width();
int app_height();

void frame()
{
    /* animate clear colors */
    float g = pass_action.colors[0].val[1] + 0.01f;
    if (g > 1.0f) g = 0.0f;
    pass_action.colors[0].val[1] = g;

    /* draw one frame */
    sg_begin_default_pass(&pass_action, app_width(), app_height());
#ifndef __EMSCRIPTEN__
    __dbgui_draw();
#endif
    sg_end_pass();
    sg_commit();
}

void init()
{
    {
        sg_desc desc{};
#ifndef __EMSCRIPTEN__
        desc.context = sapp_sgcontext();
#endif
        sg_setup(&desc);
    }

    pass_action.colors[0] = { SG_ACTION_CLEAR, {1.0f, 0.0f, 0.0f, 1.0f} };
}

#ifdef __EMSCRIPTEN__
int app_width()  { return emsc_width(); }
int app_height() { return emsc_height(); }

int main()
{
    /* setup WebGL1 context, no antialiasing */
    emsc_init("#canvas", EMSC_NONE);

    init();

    emscripten_set_main_loop(frame, 0, 1);
}
#else
int app_width()  { return sapp_width(); }
int app_height() { return sapp_height(); }

void cleanup()
{
    __dbgui_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    sapp_desc desc{};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup;
    desc.event_cb = __dbgui_event;
    desc.width = 800;
    desc.height = 600;
    desc.gl_force_gles2 = true;
    desc.window_title = "Clear (sokol app)";

    return desc;
}
#endif
