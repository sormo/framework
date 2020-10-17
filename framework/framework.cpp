#include <Windows.h>

#ifdef __EMSCRIPTEN__
#define NANOVG_GLES2_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#define NANOVG_GL3_IMPLEMENTATION
#include <glad/glad.h>
#define SOKOL_WIN32_NO_GL_LOADER
#endif

#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_fetch.h"
#include "sokol_time.h"
#include "imgui.h"
#undef SOKOL_IMPL

#include "imgui_impl.h"

#include "nanovg.h"
#include "nanovg_gl.h"

#include "framework.h"
#include "events.h"

#include "imgui_font.h"

sg_pass_action pass_action;
NVGcontext* vg;

struct
{
    float width;
    float height;
    Point position;
    NVGcolor color;

} canvas;

void canvas_setup()
{
    canvas.width = frame::screen_width();
    canvas.height = frame::screen_height();
    canvas.color = nvgRGB(0, 0, 0);
}

void canvas_apply()
{
    nvgTranslate(vg, canvas.position.x(), frame::screen_height() - canvas.position.y() - canvas.height);
    nvgBeginPath(vg);
    nvgMoveTo(vg,         0.0f,          0.0f);
    nvgLineTo(vg, canvas.width,          0.0f);
    nvgLineTo(vg, canvas.width, canvas.height);
    nvgLineTo(vg,         0.0f, canvas.height);

    nvgFillColor(vg, canvas.color);
    nvgFill(vg);
    nvgScissor(vg, 0.0f, 0.0f, canvas.width, canvas.height);
}

namespace frame
{
    void screen_background(const NVGcolor& color)
    {
        pass_action.colors[0] = { SG_ACTION_CLEAR, { color.r, color.g, color.b, color.a } };
    }

    Point rel_pos(float x, float y)
    {
        return { x * frame::canvas_width(), y * frame::canvas_height() };
    }

    float canvas_width()
    {
        return canvas.width;
    }

    float canvas_height()
    {
        return canvas.height;
    }

    void set_canvas_size(float width, float height)
    {
        canvas.width = width;
        canvas.height = height;
    }

    Point canvas_position()
    {
        return canvas.position;
    }

    void set_canvas_position(const Point& position)
    {
        canvas.position = { position.x(), position.y() };
    }

    void canvas_background(const NVGcolor& color)
    {
        canvas.color = color;
    }

    Point convert_from_gui_to_screen(const Point& position)
    {
        return { position.x(), frame::screen_height() - position.y() };
    }

    Point convert_from_canvas_to_screen(const Point& position)
    {
        return position + canvas.position;
    }

    Point convert_from_screen_to_gui(const Point& position)
    {
        return { position.x(), frame::screen_height() - position.y() };
    }

    Point convert_from_screen_to_canvas(const Point& position)
    {
        return canvas.position - position;
    }
}

char font_buffer[200000];
void font_response_callback(const sfetch_response_t* response)
{
    if (response->finished && !response->failed)
    {
        //nvgCreateFontMem(vg, "roboto", (unsigned char*)font_buffer, response->fetched_size, 0);
        nvgCreateFontMem(vg, "roboto", dump_font, sizeof(dump_font), 0);
    }
}

void frame_update()
{
    sfetch_dowork();

    imgui::prepare_render(frame::screen_width(), frame::screen_height());

    events_frame();

    sg_begin_default_pass(&pass_action, frame::screen_width(), frame::screen_height());

    nvgBeginFrame(vg, frame::screen_width(), frame::screen_height(), 1.0f);

    canvas_apply();

    update();

    nvgEndFrame(vg);

    sg_reset_state_cache();

    imgui::render();

    sg_end_pass();
    sg_commit();
}

void init()
{
#ifndef __EMSCRIPTEN__
    gladLoadGL();
#endif

    {
        sg_desc desc{};
        desc.context = sapp_sgcontext();
        sg_setup(&desc);
    }

    stm_setup();

    imgui::setup(dump_font, sizeof(dump_font));

    canvas_setup();

    pass_action.colors[0] = { SG_ACTION_CLEAR, {0.0f, 0.0f, 0.0f, 0.0f} };

#ifdef __EMSCRIPTEN__
    vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#else
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#endif

    {
        sfetch_desc_t desc{};
        sfetch_setup(&desc);
    }

    setup();

    {
        sfetch_request_t request{};
        request.path = "roboto.ttf";
        request.callback = font_response_callback;
        request.buffer_ptr = font_buffer;
        request.buffer_size = 200000;

        sfetch_send(&request);
    }
}

namespace frame
{
    float screen_width()  { return (float)sapp_width(); }
    float screen_height() { return (float)sapp_height(); }
}

void cleanup()
{
    sg_shutdown();
}

void input(const sapp_event* event)
{
    imgui::handle_event(event);
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    sapp_desc desc{};
    desc.init_cb = init;
    desc.frame_cb = frame_update;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;
    desc.width = 800;
    desc.height = 600;
    desc.gl_force_gles2 = true;
    desc.window_title = "Clear (sokol app)";

    return desc;
}
