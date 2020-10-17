#include "imgui_impl.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_time.h"
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

static uint64_t last_time = 0;

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

sg_image create_font_texture()
{
    unsigned char* font_pixels;
    int font_width, font_height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
    sg_image_desc img_desc = { };
    img_desc.width = font_width;
    img_desc.height = font_height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.min_filter = SG_FILTER_LINEAR;
    img_desc.mag_filter = SG_FILTER_LINEAR;
    img_desc.content.subimage[0][0].ptr = font_pixels;
    img_desc.content.subimage[0][0].size = font_width * font_height * 4;

    sg_image result = sg_make_image(&img_desc);
    ImGui::GetIO().Fonts->TexID = (ImTextureID)(size_t)result.id;

    return result;
}

void setup(void* fontData, size_t fontSize)
{
    simgui_desc_t simgui_desc{};
    simgui_desc.no_default_font = true;
    simgui_desc.dpi_scale = sapp_dpi_scale();
    simgui_setup(&simgui_desc);

    configure_font(fontData, fontSize);
    create_font_texture();
}

void prepare_render(float width, float height)
{
    const double delta_time = stm_sec(stm_laptime(&last_time));
    simgui_new_frame(width, height, delta_time);
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
