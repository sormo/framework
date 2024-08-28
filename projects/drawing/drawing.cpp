#include <framework.h>
#include <vector>
#include <string>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include <sokol_fetch.h>
#include <sokol_log.h>
#include <stb_image.h>
#include "imgui.h"
#include "utils.h"
#include "drawing_sg.h"
#include "svg.h"

#ifdef __EMSCRIPTEN__
#define SOKOL_GLES3
#else
#define SOKOL_GLCORE
#endif

//#define SOKOL_GL_IMPL
#include "sokol_gl.h"

#include <stdio.h>  // needed by fontstash's IO functions even though they are not used
//#define FONTSTASH_IMPLEMENTATION
//#define FONTSTASH_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#if defined(_MSC_VER )
#pragma warning(disable:4996)   // strncpy use in fontstash.h
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include "nanovg/fontstash.h"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

//#define SOKOL_FONTSTASH_IMPL
#include "sokol_fontstash.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include <HandmadeMath.h>
#undef HANDMADE_MATH_IMPLEMENTATION

#include "texrect.glsl.h"

using namespace frame;

frame::free_move_camera_config free_move_config;

using drawing_type_cbk = void(*)();

struct drawing_type_data
{
    const char* name;
    drawing_type_cbk setup;
    drawing_type_cbk update;
};

enum drawing_type
{
    instanced,
    basic,
    image,
    depth,
    text
};

void setup_instanced();
void update_instanced();
void setup_basic();
void update_basic();
void setup_image();
void update_image();
void setup_depth();
void update_depth();
void setup_text();
void update_text();

drawing_type_data drawing_types[] =
{
    { "instanced", setup_instanced, update_instanced },
    { "basic", setup_basic, update_basic },
    { "image", setup_image, update_image },
    { "depth", setup_depth, update_depth },
    { "text", setup_text, update_text }
};

frame::col4 get_random_color()
{
    return { frame::randf(), frame::randf(), frame::randf(), 1.0f };
}

frame::vec2 get_random_position()
{
    return { frame::randf(-sapp_widthf()/2.0f, sapp_widthf() / 2.0f), frame::randf(-sapp_heightf() / 2.0f, sapp_heightf() / 2.0f)};
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// COMMON /////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct
{
    frame::draw_buffer_id rectangle;
    frame::draw_buffer_id rectangle_instanced;
    frame::draw_buffer_id circle;
    frame::draw_buffer_id circle_instanced;

    std::string drawing_types_string;
    drawing_type drawing_type_current = drawing_type::basic;

} state_common;

void setup_common()
{
    state_common.rectangle = frame::create_draw_buffer("rectangle", frame::create_mesh_rectangle(), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_IMMUTABLE);
    state_common.circle = frame::create_draw_buffer("circle", frame::create_mesh_circle(60), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_IMMUTABLE);
    state_common.rectangle_instanced = frame::create_instanced_rectangle();
    state_common.circle_instanced = frame::create_instanced_circle(60);

    load_font("Regular", "DroidSerif-Regular.ttf");
    load_font("Italic", "DroidSerif-Italic.ttf");
    load_font("Bold", "DroidSerif-Bold.ttf");
    load_font("Japanese", "DroidSansJapanese.ttf");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// INSTANCED //////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct state_instanced_type
{
    struct rect
    {
        frame::vec2 position;
        float rotation;
        frame::vec2 scale;
        frame::col4 color;
        size_t index;
    };

    frame::draw_buffer_id circle_instanced_count;
    std::vector<rect> rects;

} state_instanced;

void setup_instanced()
{
    state_instanced.circle_instanced_count = frame::create_draw_buffer_instanced("circles", frame::create_mesh_circle(60), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_DYNAMIC, 256);

    for (size_t i = 0; i < 256; i++)
        frame::update_draw_instance(state_instanced.circle_instanced_count, i, get_random_position(), 0.0f, { 10.0f, 10.0f }, col4::WHITE);

    for (size_t i = 0; i < 10; i++)
        frame::add_draw_instance(state_common.circle_instanced, get_random_position(), 0.0f, { 10.0f, 10.0f }, get_random_color());

    for (size_t i = 0; i < 10; i++)
    {
        state_instanced_type::rect tr{ get_random_position(), frame::randf(0.0f, 6.28f), { 10.0f, 20.0f }, get_random_color() };
        tr.index = frame::add_draw_instance(state_common.rectangle_instanced, tr.position, tr.rotation, tr.scale, get_random_color());

        state_instanced.rects.push_back(std::move(tr));
    }
}

void update_instanced()
{
    for (auto& tr : state_instanced.rects)
    {
        tr.rotation += 0.05f;
        frame::update_draw_instance(state_common.rectangle_instanced, tr.index, tr.position, tr.rotation, tr.scale, tr.color);
    }

    //sg_range update_range;
    //update_range.ptr = state_text.rect.array;
    //update_range.size = sizeof(instanced_element) * state.rect.instances;

    //sg_update_buffer(state.rect.bind.vertex_buffers[1], &update_range);

    frame::draw_buffer_instanced(state_common.rectangle_instanced);
    //frame::draw_buffer_instanced(state.circle);
    frame::draw_buffer_instanced(state_instanced.circle_instanced_count, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// BASIC //////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct
{
    frame::draw_buffer_id polyline;
    frame::draw_buffer_id center;

} state_basic;

void setup_basic()
{
    std::vector<frame::vec2> vertices{ {0.0f, 0.0f}, {100.0f, 100.0f}, {-100.0f, 100.0f}, {0.0f, 0.0f} };

    state_basic.polyline = frame::create_draw_buffer("polyline", { (float*)vertices.data(), vertices.size(), nullptr, 0 }, sg_primitive_type::SG_PRIMITIVETYPE_LINE_STRIP, sg_usage::SG_USAGE_DYNAMIC);
    state_basic.center = frame::create_draw_buffer("center", frame::create_mesh_rectangle(), sg_primitive_type::SG_PRIMITIVETYPE_TRIANGLE_STRIP, sg_usage::SG_USAGE_DYNAMIC);

}

void update_basic()
{
    auto text_center = vec2(100.0f, 100.0f);
    auto text_align = text_align::top_middle;
    auto text_size = 15.0f;
    auto text = "test text first line\nsecond line";
    frame::draw_circle(text_center, 3.0f / frame::get_world_scale().x, col4::RED);

    auto rect = frame::get_text_rectangle(text, text_center, text_size, text_align);
    frame::draw_rectangle(rect.center(), rect.size().x, rect.size().y, frame::col4::ORANGE);
    frame::draw_text(text, text_center, text_size, col4::BLACK, text_align);

    //frame::draw_circle({ 200.0f, 200.0f }, 3.0f, col4::RED);

    {
        auto position = vec2(200.0f, 200.0f);
        auto rect = frame::get_text_rectangle("Hello, World", position, 20.0f, frame::text_align::bottom_left);

        frame::save_world_transform();
        frame::set_world_transform(frame::identity());
        auto rect2 = frame::get_text_rectangle("Hello, World", {}, 20.0f);
        frame::restore_world_transform();

        rect2.max /= get_world_scale();
        rect2.min += position;
        rect2.max += position;

        // the rectangle has top_left align (means [0,0] is top left)
        vec2 align_modif(0.0f, rect2.size().y);

        std::swap(rect2.min.y, rect2.max.y);

        rect2.min -= align_modif;
        rect2.max -= align_modif;

        frame::draw_rectangle(rect, frame::col4::BLUE);
        frame::draw_rectangle(rect2, frame::col4::GREEN);
        frame::draw_text("Hello, World", position, 20.0f, frame::col4::RED, frame::text_align::bottom_left);
    }

    frame::draw_buffer(state_basic.polyline, { 200.0f, 200.0f }, 90.0f, { 0.5f, 0.5f }, frame::col4::WHITE);
    //frame::draw_buffer(state.center, frame::translation({ 0.0f, 0.0f }) * frame::scale({ 200.0f, 200.0f }), frame::col4::WHITE);
    frame::draw_buffer(state_basic.center, frame::translation({ 0.0f, 0.0f }) * frame::scale({ 5.0f, 5.0f }), frame::col4::WHITE);

    {
        auto draw_text = [](const char* text, const vec2& pos, frame::text_align align, col4 colorRect)
        {
            const float font_size = 40.0f;
            auto rect = get_text_rectangle2(text, pos, font_size, "Regular", align);
            frame::draw_circle(pos, 3.0f / get_world_scale().x, col4::RED);
            frame::draw_buffer(state_common.rectangle, { rect.center(), -1.0f }, 0.0f, rect.size(), colorRect);
            draw_text_ex2(text, vec3(pos, 1.0f), font_size, col4::BLACK, "Regular", align);
        };

        draw_text("TopLeft", { -200.0f, -200.0f }, text_align::top_left, col4::ORANGE);
        draw_text("MiddleMiddle", { -200.0f, -100.0f }, text_align::middle_middle, col4::EARTHBLUE);
        draw_text("BottomRight", { -200.0f, 0.0f }, text_align::bottom_right, col4::GOLD);
    }

    {
        float y = 50.0f;
        {
            static float x = -300.0f;
            static float speed = 0.01f;
            x += speed;

            frame::draw_text_ex2("TEST", { x, y, 0.0f }, 30.0f, col4::BLACK, "Regular", text_align::bottom_left);
            if (x > 300.0f || x < -300.0f)
                speed = -speed;
        }
        {
            static float x = 300.0f;
            static float speed = -0.01f;
            x += speed;

            frame::draw_text_ex("TEST", { x, y }, 30.0f, col4::WHITE, "Regular", text_align::bottom_left);
            if (x > 300.0f || x < -300.0f)
                speed = -speed;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// IMAGE //////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct
{
    frame::svg_image* svg_test;
    frame::image image_test;
    frame::image rasterize_test;
    frame::svg_image* rasterize_test_svg;

    sg_pipeline test_sg_image_pip;
    sg_bindings test_sg_image_bind;

} state_image;

void setup_test_sg_image(const char* sokol_image_base64)
{
    struct vertex_t
    {
        float x, y;
        uint16_t u, v;
    };

    vertex_t vertices[] =
    {
         0.5f,  0.5f, 32767, 32767,
         0.5f, -0.5f, 32767,     0,
        -0.5f, -0.5f,     0,     0,
        -0.5f,  0.5f,     0, 32767
    };

    sg_buffer_desc buffer_desc_vert = {};
    buffer_desc_vert.data = SG_RANGE(vertices);
    buffer_desc_vert.label = "texrect-vertices";
    state_image.test_sg_image_bind.vertex_buffers[0] = sg_make_buffer(&buffer_desc_vert);

    uint16_t indices[] = { 0, 1, 3, 2 };
    sg_buffer_desc buffer_desc_index = {};
    buffer_desc_index.type = SG_BUFFERTYPE_INDEXBUFFER;
    buffer_desc_index.data = SG_RANGE(indices);
    buffer_desc_index.label = "texrect-indices";
    state_image.test_sg_image_bind.index_buffer = sg_make_buffer(&buffer_desc_index);

    auto swap = [](uint32_t n) { return ((n >> 24) & 0xff) | ((n << 8) & 0xff0000) | ((n >> 8) & 0xff00) | ((n << 24) & 0xff000000); };

    // create a checkerboard texture
    uint32_t pixels[4 * 4] =
    {
        0xFF0000FF, 0xFF000000, 0xFFFFFFFF, swap(frame::col4::BLUE.to_hex()),
        0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
        0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
        0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
    };

    auto sokol_image = frame::base64_decode(sokol_image_base64);
    int w, h, n, image;
    unsigned char* img = stbi_load_from_memory((const unsigned char*)sokol_image.data(), sokol_image.size(), &w, &h, &n, 4);

    sg_image_desc image_desc = {};
    image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    //image_desc.width = image_desc.height = 4;
    image_desc.width = w;
    image_desc.height = h;
    //image_desc.data.subimage[0][0] = SG_RANGE(pixels);
    image_desc.data.subimage[0][0] = sg_range{ (const void*)img, (size_t)(w * h * 4) };
    image_desc.label = "texrect-texture";
    state_image.test_sg_image_bind.fs.images[SLOT_tex] = sg_make_image(&image_desc);

    stbi_image_free(img);

    // create a sampler object with default attributes
    sg_sampler_desc sampler_desc = {};
    sampler_desc.wrap_u = sampler_desc.wrap_v = SG_WRAP_CLAMP_TO_BORDER;
    sampler_desc.min_filter = sampler_desc.mag_filter = SG_FILTER_LINEAR;
    state_image.test_sg_image_bind.fs.samplers[SLOT_smp] = sg_make_sampler(&sampler_desc);

    // a shader
    sg_shader shd = sg_make_shader(texrect_shader_desc(sg_query_backend()));

    // a pipeline state object
    sg_pipeline_desc pipeline_desc = {};
    pipeline_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    pipeline_desc.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
    pipeline_desc.layout.attrs[ATTR_vs_texcoord0].format = SG_VERTEXFORMAT_SHORT2N;
    pipeline_desc.shader = shd;
    pipeline_desc.alpha_to_coverage_enabled = true;
    pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
    pipeline_desc.label = "texrect-pipeline";
    state_image.test_sg_image_pip = sg_make_pipeline(&pipeline_desc);
}

void setup_image()
{
    static const char earth_svg[] = R"(<?xml version="1.0" encoding="UTF-8"?>
                                       <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 12 12" version="1.1" style="fill:none;stroke-width:0.6;stroke:#ffffff">
	                                       <circle cx="6" cy="6" r="5"/>
	                                       <path d="M 6 1 L 6 11" />
	                                       <path d="M 1 6 L 11 6" />
                                       </svg>)";

    static const char sun_svg[] = R"(<?xml version="1.0" encoding="UTF-8"?>
                                     <svg viewBox="0 0 50 50" xmlns="http://www.w3.org/2000/svg" style="fill:#ffffff>
                                      <g stroke-width="none">
                                       <title>U+1F31E SUN WITH FACE</title>
                                       <path d="m25 8q3.2483-1.7203 2.0584-3.8989-1.4939-2.0942 2.8189-3.6208-0.39018 1.9616 1.2187 4.4239 1.175 2.542 0.40964 4.3898 3.6593-0.34627 3.3937-2.8144-0.57878-2.5065 3.9899-2.2664-1.1111 1.6629-0.567 4.5535 0.11277 2.7982-1.3014 4.2124 3.5133 1.0804 4.2124-1.3014 0.42446-2.5372 4.5535-0.567-1.6629 1.1111-2.2664 3.9899-0.96664 2.6283-2.8144 3.3937 2.8324 2.3427 4.3898 0.40964 1.3631-2.1816 4.4239 1.2187-1.9616 0.39018-3.6208 2.8189-1.8989 2.0584-3.8989 2.0584 1.7203 3.2483 3.8989 2.0584 2.0942-1.4939 3.6208 2.8189-1.9616-0.39018-4.4239 1.2187-2.542 1.175-4.3898 0.40964 0.34627 3.6593 2.8144 3.3937 2.5065-0.57878 2.2664 3.9899-1.6629-1.1111-4.5535-0.567-2.7982 0.11277-4.2124-1.3014-1.0805 3.5133 1.3014 4.2124 2.5372 0.42446 0.567 4.5535-1.1111-1.6629-3.9899-2.2664-2.6283-0.96664-3.3937-2.8144-2.3427 2.8324-0.40964 4.3898 2.1816 1.3631-1.2187 4.4239-0.39018-1.9616-2.8189-3.6208-2.0584-1.8989-2.0584-3.8989-3.2483 1.7203-2.0584 3.8989 1.4939 2.0942-2.8189 3.6208 0.39018-1.9616-1.2187-4.4239-1.175-2.542-0.40964-4.3898-3.6593 0.34627-3.3937 2.8144 0.57878 2.5065-3.9899 2.2664 1.1111-1.6629 0.567-4.5535-0.11277-2.7982 1.3014-4.2124-3.5133-1.0805-4.2124 1.3014-0.42446 2.5372-4.5535 0.567 1.6629-1.1111 2.2664-3.9899 0.96664-2.6283 2.8144-3.3937-2.8324-2.3427-4.3898-0.40964-1.3631 2.1816-4.4239-1.2187 1.9616-0.39018 3.6208-2.8189 1.8989-2.0584 3.8989-2.0584-1.7203-3.2483-3.8989-2.0584-2.0942 1.4939-3.6208-2.8189 1.9616 0.39018 4.4239-1.2187 2.542-1.175 4.3898-0.40964-0.34627-3.6593-2.8144-3.3937-2.5065 0.57878-2.2664-3.9899 1.6629 1.1111 4.5535 0.567 2.7982-0.11277 4.2124 1.3014 1.0804-3.5133-1.3014-4.2124-2.5372-0.42446-0.567-4.5535 1.1111 1.6629 3.9899 2.2664 2.6283 0.96664 3.3937 2.8144 2.3427-2.8324 0.40964-4.3898-2.1816-1.3631 1.2187-4.4239 0.39018 1.9616 2.8189 3.6208 2.0584 1.8989 2.0584 3.8989m0 2a15 15 0 0 0 0 30 15 15 0 0 0 0-30"/>
                                       <path d="m25 22q-3 0-6-3t-9 3q6-3 9 0 2 2 6 0 3 0 6-3t9 3q-6-3-9 0-2 2-6 0"/>
                                       <path d="m16.5 19a2.5 2.5 0 0 0 0 5 2.5 2.5 0 0 0 0-5m17 0a2.5 2.5 0 0 0 0 5 2.5 2.5 0 0 0 0-5"/>
                                       <path d="m22 25a4 4 0 0 0 6 0 3 4 0 1 1-6 0"/>
                                       <path d="m16 29a20 20 0 0 0 18 0 10 10 0 0 1-18 0"/>
                                      </g>
                                     </svg>)";

    static const char sokol[] = "iVBORw0KGgoAAAANSUhEUgAAA5gAAACwCAYAAABq38F7AAAAAXNSR0IArs4c6QAACVxJREFUeJzt3d+"
        "LnFcZB/CZss0mJiHJNkbFWg2UUKNWQ0GpIJIIJhZ/Ia1Y4520hUIpiEW8C94UaS9aFGmkKogh0kapVs"
        "SaJilVkEjboFGMunQDJtLuprMxP5psYjL+A5t5hfM9vDOzn8/t5Jz3cN5nntlvzsXp9r/7wU5V0+v7d"
        "R/AIFM/OTrw896ud3SLHnDDhaLhrXtjRd351f9AI1+fh99Vd/5OR/0M0Fg/37iprH5qO7lmpN9v9f2/"
        "8UzR8Eb6f6v0/0bqp0WN9fmVDxTVZ/exAyXDh951bS8AAACA8SBgAgAAECFgAgAAECFgAgAAECFgAgA"
        "AECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAg"
        "AAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAENHt793Ub3sRJ"
        "aYeON32Eor0vrO27SUUadr/3q/f2y16wKrlRcMb/WlG/Q8w9vX54pay+vz7sqLhTabuOVy1Psf+/f70"
        "Q2Xvt7a542Pdf3pPbC7b/82TRcMb6f8DjX1/0P9rTl9d23//z5+aHe7fl5Y5wQQAACBCwAQAACBCwAQ"
        "AACBCwAQAACBCwAQAACBCwAQAACBCwAQAACBiou0F1Nbb9b6ye2pevxRaybXMjfQ9XAy33tc2Ft7TNK"
        "8+W9T7dmH/anTS+4Uxpf+PNv2/TPH+Ha17Btd97EDV+dvmBBMAAIAIARMAAIAIARMAAIAIARMAAIAIA"
        "RMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAI"
        "ARMAAICIbr/3pX7biygxtengwM//8vuPdEvmX7h0pWR4o9u2vTTS+99k/tRs0f6Pe33W1rT//b2bBo6"
        "feuB01f3v/WNbzemra3q/x45uLar/ybnTJcMbrblx3Vh/v4794fai/d9ww4qx3p/a9P/h3n/9v4z+36"
        "6m/Z85uKVo/9fe+lzJ8CXPCSYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARE"
        "20vYKk79swtRff0NJl882LN6Tudt6+rO/+Y+9ezZe9/5fHXGv7F2pLpi9f3fxjpe7hglPWeek/R9/s/"
        "6/X/Evq//g/jygkmAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQI"
        "mAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAERNTmw5WfcDvXt7a6vzznU6/6A"
        "ErikY3enPN6qrzf+y2QwM//8GhO7ol869fdrVkeKOpswtV52/bP992U9H4Lbe/GFrJ4o68+uGq819/9"
        "nLV+dv27wvXl02w6q2ZhVzDqO9/U/+fK+z/c2fLfj6aNPXn2l559y1F46/r1u3/o16fTfT/8X6/+n+7"
        "ZirvX6knX7qn7SVU5QQTAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACAiImZv24"
        "tugexyYmLhfdQAsASdODw9qLf536n6s87ACzKCSYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAi"
        "YAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARE"
        "99/dVXVBzz8mWerzj/qHvnN51qdv7fQ6ZfM/9Wtvxj8/Oc+3S2Zv3N2smj4sHthdnnR+Hv33x9ayeJe"
        "mL1Udf7VEyuqzt+21y+W/R/eiQsToZVcQ3+09/+hHYP7T6m2+/Mr82X9uWl/7tv3haL+fPPquv1h1Ou"
        "zif4/3u9X/2/Xy/PLisbX/n5Nn7tadf6bV10p+v1o0vT74gQTAACACAETAACACAETAACACAETAACACA"
        "ETAACACAETAACACAETAACAiMqX7HQ6j/72jrJ7EBu8c8XlmtN3Ll2tm8FnL5TdcwbA4n54aEfR788bh"
        "fcEA8BS5AQTAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACA"
        "CAETAACACAETAACACAETAACACAETAACACAETAACACAETAACAiInpc5P9mg+YPldz9k7n6bv3Dfz8o99"
        "7vlsy/9rJ2ZLhjVa+5edV529bb2GiaPzDn30mtJLF3bX3zlbn/9vZTtH3r6n+P757b1H9dzo/q9ofam"
        "van9p2bP5xq89vcu/++9teQlWH595fNH73Fx8NrWRxbfef3uWy/vPQ9l8N/Hznnl1F/WfPzl0lwxu1v"
        "f/6f136/2Dj3v8Pnfhy0fjTCxtCK7mGdU9U/X79cb7dM0QnmAAAAEQImAAAAEQImAAAAEQImAAAAEQI"
        "mAAAAEQImAAAAEQImAAAAESUXVIIlX3zl58vvMdrsOnzZfeQMdpK6+vkme2ppSzqzH+rln+n03m+8vy"
        "j7b6nvl71BfSuHNd/BtD/qUn/1/+pxwkmAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQ"
        "ImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEd116zf0a"
        "z5gy+MzNaenwZEHN7b6/PlTs91WF9BA/ber7fq8a++dRfU5PftIainXUrU+R11T/Xziyd1D3X965z85"
        "0u+39v5PrdxfMrzR03fv0/9bpP83Gun+wHhr+v46wQQAACBCwAQAACBCwAQAACBCwAQAACBCwAQAACB"
        "CwAQAACBCwAQAACBi4lM/+lbVe8JeO+Men2FW+v5XLf9zaimtOHfxVvU/xJZ6fQL1+PtnuOn/MLqcYA"
        "IAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhY"
        "AIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABDR3bbnfNUHHHlwY7/qAyiy5fGZbsn4gztXppbSCvU/"
        "3OZPzRbVZ23qZ7g11U/t99dk3N+v7+94v9/a1I/6YXQ5wQQAACBCwAQAACBCwAQAACBCwAQAACBCwAQ"
        "AACBCwAQAACBCwAQAACDifxFf+7S1xG8ZAAAAAElFTkSuQmCC)";

    static const char menu_svg[] = R"(<svg width="800px" height="800px" fill="none" version="1.1" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                                       <g id="a">
                                        <g id="b" clip-rule="evenodd" fill="#c8c8c8" fill-rule="evenodd">
                                         <path id="c" d="m2.75 12c0-1.2426 1.0074-2.25 2.25-2.25s2.25 1.0074 2.25 2.25-1.0074 2.25-2.25 2.25-2.25-1.0074-2.25-2.25z"/>
                                         <path d="m9.75 12c0-1.2426 1.0074-2.25 2.25-2.25s2.25 1.0074 2.25 2.25-1.0074 2.25-2.25 2.25-2.25-1.0074-2.25-2.25z"/>
                                         <path d="m16.75 12c0-1.2426 1.0074-2.25 2.25-2.25s2.25 1.0074 2.25 2.25-1.0074 2.25-2.25 2.25-2.25-1.0074-2.25-2.25z"/>
                                        </g>
                                       </g>
                                      </svg>)";

    state_image.image_test = frame::image_create(frame::base64_decode(sokol));
    state_image.svg_test = frame::svg_parse(sun_svg);

    state_image.rasterize_test_svg = frame::svg_parse(menu_svg);
    state_image.rasterize_test = frame::svg_rasterize(state_image.rasterize_test_svg, 16, 16);

    setup_test_sg_image(sokol);
}

void update_image()
{
    //frame::draw_svg(svg_test, {10.0f, 10.0f}, frame::text_align::middle_middle);
    //frame::draw_svg_ex(svg_test, { 10.0f, 10.0f }, 45.0f, { 1.0f, 1.0f }, frame::text_align::middle_middle);
    //frame::draw_svg_ex_size(svg_test, { 10.0f, -10.0f }, 0.0f, { 20.0f, 20.0f }, frame::text_align::middle_middle);
    frame::draw_svg_ex_size(state_image.svg_test, { 10.0f, 10.0f }, 90.0f, { 80.0f, 80.0f }, frame::text_align::bottom_right);

    auto image_size = frame::get_image_size(state_image.image_test);
    frame::draw_circle({ 100.0f, 100.0f }, 3.0f, col4::RED);
    frame::draw_rectangle(frame::rectangle::from_min_max({ 100.0f, 100.0f }, { 300.0f, 50.0f }), col4::BLUE);

    frame::draw_image(state_image.image_test, { 100.0f, 100.0f }, frame::text_align::top_left);
    //frame::draw_image_ex(state_image.image_test, { 100.0f, 100.0f }, frame::deg_to_rad(45.0f), {0.2f, 0.2f}, text_align::top_middle);
    //frame::draw_image_ex_size(state_image.image_test, { 100.0f, 100.0f }, 0.0f, image_size * 0.3f, text_align::top_left);
    //frame::draw_image_ex_size(state_image.image_test, { 100.0f, 100.0f }, frame::deg_to_rad(90.0f), { 200.0f, 50.0f }, text_align::top_left);

    frame::draw_image(state_image.rasterize_test, { -200, 200 });
    //frame::draw_svg(rasterize_test_svg, { -200, 200 });

    vs_params_t vs_params;
    vs_params.color0[0] = vs_params.color0[1] = vs_params.color0[2] = vs_params.color0[3] = 1.0f;
    vs_params.mvp = HMM_MulM4(create_projection_view_matrix(), ::create_hmm_transform(vec2{}, 0.0f, { 500.0f, 50.0f }));

    sg_apply_pipeline(state_image.test_sg_image_pip);
    sg_apply_bindings(&state_image.test_sg_image_bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(vs_params));
    sg_draw(0, 4, 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// DEPTH //////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct
{
    frame::draw_buffer_id rectangle_depth;

} state_depth;

void setup_depth()
{
    auto add_instances = [](col4 color, float depth)
        {
            for (size_t i = 0; i < 60; i++)
            {
                auto pos = get_random_position();
                frame::add_draw_instance(state_common.circle_instanced, { pos.x, pos.y, depth }, 0.0f, { 80.0f, 80.0f }, color);
            }
        };

    add_instances(col4::EARTHBLUE, -1.0f);
    add_instances(col4::GOLD, 1.0f);

    frame::mesh_data mesh;
    // vertices with varying depth, starts at 1.5f on right side and drops to 0.5f on left side
    mesh.vertices =
    {
         0.5f,  0.5f, 1.5f,
         0.5f, -0.5f, 1.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f,  0.5f, 0.5f
    };
    mesh.indices = { 0, 1, 3, 2 };
    mesh.type = mesh_t::depth;

    state_depth.rectangle_depth = frame::create_draw_buffer("rectangle-depth", mesh, SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_IMMUTABLE);
}

void update_depth()
{
    frame::draw_buffer_instanced(state_common.circle_instanced);

    frame::draw_buffer(state_common.rectangle, { 0.0f, 0.0f, 10.0f }, 0.0f, { 60.0f, 60.0f }, col4::ORANGE);
    frame::draw_buffer(state_common.rectangle, { 0.0f, 0.0f, 0.0f }, 0.0f, { 80.0f, 80.0f }, col4::DARKGRAY);
    frame::draw_buffer(state_common.rectangle, { 0.0f, 0.0f, -100.0f }, 0.0f, { 100.0f, 100.0f }, col4::BLUE);

    frame::draw_buffer(state_depth.rectangle_depth, { -150.0f, -100.0f }, 0.0f, { 250.0f, 200.0f }, col4::RGB(0, 255, 12, 128));

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////// TEXT ///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void line(float sx, float sy, float ex, float ey)
{
    frame::save_world_transform();
    frame::set_world_transform(frame::translation(frame::get_world_translation()));
    frame::draw_line_solid({ sx, sy }, { ex, ey }, col4::YELLOW);
    frame::restore_world_transform();
}

void setup_text()
{
    set_screen_background(col4::DARKGRAY);
}

void update_text()
{
    //set_world_transform(frame::identity());

    float dpis = sapp_dpi_scale();
    float sx, sy = 0.0f;
    auto white = col4::RGB(255, 255, 255, 255);
    auto black = col4::RGB(0, 0, 0, 255);
    auto brown = col4::RGB(192, 128, 0, 128);
    auto blue = col4::RGB(0, 192, 255, 255);

    // Quick fox

    sx = 50 * dpis; sy = 50 * dpis;
    
    vec2 d(sx, sy);
    auto sc = get_world_scale();

    d.y += get_font_metrics2("Regular", 124.0f).line_height;
    d.x = draw_text_ex2("The quick ", { d / sc, 0.0f }, 124.0f, white, "Regular", text_align::baseline_left);
    d.x = draw_text_ex2("brown ", { d / sc, 0.0f }, 48.0f, brown, "Italic", text_align::baseline_left);
    d.x = draw_text_ex2("fox ", { d / sc, 0.0f }, 24.0f, white, "Regular", text_align::baseline_left);

    d.x = sx;
    d.y += get_font_metrics2("Regular", 24.0f).line_height * 1.2f;

    d.x = draw_text_ex2("jumps over ", { d / sc, 0.0f }, 24.0f, white, "Italic", text_align::baseline_left);
    d.x = draw_text_ex2("the lazy ", { d / sc, 0.0f }, 24.0f, white, "Bold", text_align::baseline_left);
    d.x = draw_text_ex2("dog.", { d / sc, 0.0f }, 24.0f, white, "Regular", text_align::baseline_left);

    d.x = sx;
    d.y += get_font_metrics2("Regular", 24.0f).line_height * 1.2f;
    draw_text_ex2("Now is the time for all good men to come to the aid of the party.", { d / sc, 0.0f }, 12.0f, blue, "Regular", text_align::baseline_left);

    // Special characters
    // TODO doesn't work

    d.x = sx;
    d.y += get_font_metrics2("Regular", 24.0f).line_height * 1.5f;
    draw_text_ex2("Ég get etið gler án þess að meiða mig.", { d / sc, 0.0f }, 18.0f, white, "Italic", text_align::baseline_left);

    d.x = sx;
    d.y += get_font_metrics2("Italic", 18.0f).line_height * 1.2f;
    draw_text_ex2("私はガラスを食べられます。それは私を傷つけません。", { d / sc, 0.0f }, 18.0f, white, "Japanese", text_align::baseline_left);

    // Allignment

    d.x = 50 * dpis; d.y = 350 * dpis;
    line(d.x - 10 * dpis, d.y, d.x + 250 * dpis, d.y);

    d.x = draw_text_ex2("Top", { d / sc, 0.0f }, 18.0f, white, "Regular", text_align::top_left);
    d.x += 10 * dpis;
    d.x = draw_text_ex2("Middle", { d / sc, 0.0f }, 18.0f, white, "Regular", text_align::middle_left);
    d.x += 10 * dpis;
    d.x = draw_text_ex2("Baseline", { d / sc, 0.0f }, 18.0f, white, "Regular", text_align::baseline_left);
    d.x += 10 * dpis;
    draw_text_ex2("Bottom", { d / sc, 0.0f }, 18.0f, white, "Regular", text_align::bottom_left);
    
    d.x = 150 * dpis; d.y = 400 * dpis;
    line(d.x, d.y - 30 * dpis, d.x, d.y + 80.0f * dpis);

    draw_text_ex2("Left", { d / sc, 0.0f }, 18.0f, white, "Regular", text_align::baseline_left);
    d.y += 30 * dpis;
    draw_text_ex2("Middle", { d / sc, 0.0f }, 18.0f, white, "Regular", text_align::baseline_middle);
    d.y += 30 * dpis;
    draw_text_ex2("Right", { d / sc, 0.0f }, 18.0f, white, "Regular", text_align::baseline_right);

    // Blur

    d.x = 500 * dpis; d.y = 350 * dpis;
    draw_text_ex2("Blurry...", { d / sc, 0.0f }, 60.0f, white, "Italic", text_align::baseline_left, 10.0f, 5.0f * dpis);

    d.y += 50.0f * dpis;
    draw_text_ex2("DROP THAT SHADOW", { d.x / sc.x, (d.y + 2.0f) / sc.y + 2.0f, 0.0f }, 18.0f, black, "Bold", text_align::baseline_left, 3.0f);
    draw_text_ex2("DROP THAT SHADOW", { d / sc, 0.0f }, 18.0f, white, "Bold", text_align::baseline_left);

    //auto position = vec3(100.0f, -100.0f, 1.0f);
    //auto align = text_align::bottom_left;
    //
    //auto rect = frame::get_text_rectangle2("Draw Depth", position, 120.0f, "Regular", align);
    //frame::draw_buffer(state_common.rectangle, { rect.center().x, rect.center().y, -1.0f}, 0.0f, rect.size() / get_world_scale(), col4::ORANGE);
    //frame::draw_text_ex2("Draw Depth", position, 120.0f, col4::WHITE, "Regular", align);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
    setup_common();

    for (auto& m : drawing_types)
    {
        m.setup();
        state_common.drawing_types_string += m.name;
        state_common.drawing_types_string.push_back('\0');
    }
    state_common.drawing_types_string.push_back('\0');

    frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, 1.0f }));
    free_move_config.min_size = { 0.1f, 0.1f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 1'000'000.0f, 1'000'000.0f });
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

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", frame::get_world_size().x, frame::get_world_size().y);

    ImGui::EndMainMenuBar();

    bool open = true;
    ImGui::SetNextWindowPos({ 0.0f, 20.0f });
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::Combo("Type", (int*)&state_common.drawing_type_current, state_common.drawing_types_string.data());

    ImGui::End();
}

void update()
{
    update_imgui();

    drawing_types[state_common.drawing_type_current].update();

    frame::free_move_camera_update(free_move_config);
}
