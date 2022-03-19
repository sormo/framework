#include "imgui_impl.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_time.h"
#include "imgui.h"

#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

static sg_pass_action pass_action;

namespace imgui
{
void configure_font(void* data, size_t size)
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x007e, // Basic Latin
        0x00a0, 0x02d9, // ISO/IEC 8859-2 (Latin-2)
        0,
    };

    auto& io = ImGui::GetIO();
    ImFontConfig fontCfg;
    fontCfg.FontDataOwnedByAtlas = false;
    fontCfg.OversampleH = 2;
    fontCfg.OversampleV = 2;
    fontCfg.RasterizerMultiply = 1.5f;
    fontCfg.GlyphRanges = ranges;
    io.Fonts->AddFontFromMemoryTTF(data, size, 16.0f, &fontCfg);
}

void setup(void*, size_t)
{
    // setup sokol-gfx, sokol-time and sokol-imgui
    sg_desc desc = { };
    desc.context = sapp_sgcontext();
    sg_setup(&desc);

    // use sokol-imgui with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    simgui_desc_t simgui_desc = { };
    simgui_setup(&simgui_desc);

    // initial clear color
    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.colors[0].value = { 0.0f, 0.5f, 0.7f, 1.0f };
}

void prepare_render()
{
    const int width = sapp_width();
    const int height = sapp_height();

    simgui_new_frame({ width, height, sapp_frame_duration(), sapp_dpi_scale() });
}

void render()
{
    simgui_render();
}

void handle_event(const sapp_event* event)
{
    simgui_handle_event(event);
}
}
