#ifdef __EMSCRIPTEN__
#define NANOVG_GLES2_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#define NOMINMAX
#include <Windows.h>
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
#include "sokol_log.h"
#include "imgui.h"
#undef SOKOL_IMPL

#include "imgui_impl.h"

#include "nanovg.h"
#include "nanovg_gl.h"

#include "framework.h"
#include "events.h"

#include "imgui_font.h"
#include <chrono>
#include <vector>
#include <algorithm>

using namespace frame;

sg_pass_action pass_action;
NVGcontext* vg;
col4 background_color;

std::chrono::time_point<std::chrono::high_resolution_clock> start_application;
std::chrono::time_point<std::chrono::high_resolution_clock> start_frame;
float frame_delta = 0.0f;

// TODO store also lazily inversion
std::vector<mat3> transforms;

void apply_transform(const mat3& m)
{
    nvgTransform(vg, m.data[0], m.data[3], m.data[1], m.data[4], m.data[2], m.data[5]);
}

namespace frame
{
    void setup_draw_sg();

    col4 rgb(char r, char g, char b)
    {
        return col4::RGB(r, g, b);
    }

    col4 rgba(char r, char g, char b, char a)
    {
        return col4::RGB(r, g, b, a);
    }

    float deg_to_rad(float deg)
    {
        return deg * 0.0174533f;
    }

    float rad_to_deg(float rad)
    {
        return rad * 57.2958f;
    }

    void set_screen_background(const col4& color)
    {
        pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
        pass_action.colors[0].clear_value = { color.data.r, color.data.g, color.data.b, color.data.a };

        background_color = color;;
    }

    const col4& get_screen_background()
    {
        return background_color;
    }

    vec2 get_screen_size()
    {
        return { (float)sapp_width(), (float)sapp_height() };
    }

    float get_delta_time()
    {
        return frame_delta;
    }

    float get_time()
    {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_application).count();
    }

    mat3 translation(const vec2& translation)
    {
        return mat3::translation(translation);
    }

    mat3 rotation(float rotation)
    {
        return mat3::rotation(rotation);
    }

    mat3 scale(const vec2& scale)
    {
        return mat3::scaling(scale);
    }

    mat3 identity()
    {
        return mat3::identity();
    }

    void set_world_transform(const mat3& transform)
    {
        if (transforms.size() == 1)
            transforms.push_back(transform);
        else
            transforms.back() = transform;

        nvgResetTransform(vg);
        apply_transform(transform);
    }

    void set_world_transform_multiply(const mat3& transform)
    {
        transforms.back() = transform * transforms.back();
        apply_transform(transform);
    }

    void save_world_transform()
    {
        transforms.push_back(transforms.back());
    }

    void restore_world_transform()
    {
        assert(transforms.size());
        if (transforms.size() == 1) // we do not pop first identity matrix
            return;

        transforms.pop_back();

        nvgResetTransform(vg);
        apply_transform(transforms.back());
    }

    const mat3& get_world_transform()
    {
        return transforms.back();
    }

    vec2 get_screen_to_world(const vec2& screen_position)
    {
        return transforms.back().inverted().transform_point(screen_position);
    }

    vec2 get_mouse_world_position()
    {
        return transforms.back().inverted().transform_point(get_mouse_screen_position());
    }

    vec2 get_world_position_screen_relative(const vec2& rel)
    {
        auto world_rect = get_world_rectangle();

        return world_rect.min + world_rect.size() * rel;
    }

    vec2 get_world_size()
    {
        auto result = get_screen_size() / transforms.back().get_scale();
        return { std::abs(result.x), std::abs(result.y) };
    }

    vec2 get_world_translation()
    {
        return transforms.back().get_translation();
    }

    void set_world_translation(const vec2& translation)
    {
        transforms.back().set_translation(translation);

        nvgResetTransform(vg);
        apply_transform(transforms.back());
    }

    void set_world_translation(const vec2& screen_point, const vec2& world_point)
    {
        // find new translation such that world_point is transformed to screen_point
        //
        // |a b c|   |wx|   |sx|
        // |d e f| * |wy| = |sy|
        // |0 0 1|   | 1|   | 1|
        // 
        // we need to find new c,f (translation) such that {wx,wy} is known world_point and {sx,sy} is known screen_point
        // sx = a*wx + b*wy + c
        // sy = d*wx + e*wy + f
        // then
        // c = sx - a*wx - b*wy
        // f = sy - d*wx - e*wy

        const auto& M = transforms.back().data;

        float c = screen_point.x - M[0] * world_point.x - M[1] * world_point.y;
        float f = screen_point.y - M[3] * world_point.x - M[4] * world_point.y;

        set_world_translation({ c,f });
    }

    vec2 get_world_scale()
    {
        return transforms.back().get_scale();
    }

    void set_world_scale(const vec2& scale)
    {
        transforms.back().set_scale(scale);
        set_world_transform(transforms.back());
    }

    void set_world_scale(const vec2& scale, const vec2& stationary_world_point)
    {
        mat3 new_transform = mat3::scaling(scale);
        // find new translation such that we will preserve stationary_world_point(after scale)
        // what we need to achieve is that current stationary screen position s maps to same world position w (as with current transform)
        // M * w = s
        //
        // |a b c|   |wx|   |sx|
        // |d e f| * |wy| = |sy|
        // |0 0 1|   | 1|   | 1|
        //
        // we need to find new c,f (translation) for this equation to hold
        // sx = a*wx + b*wy + c
        // sy = d*wx + e*wy + f
        {
            vec2 s = transforms.back().transform_point(stationary_world_point);
            const vec2& w = stationary_world_point;
            float c = s.x - new_transform.data[0] * w.x - new_transform.data[1] * w.y;
            float f = s.y - new_transform.data[3] * w.x - new_transform.data[4] * w.y;

            new_transform.set_translation({ c,f });
        }
        set_world_transform(new_transform);
    }

    rectangle get_world_rectangle()
    {
        vec2 p1 = transforms.back().inverted().transform_point(vec2{ 0.0,0.0 });
        vec2 p2 = transforms.back().inverted().transform_point(get_screen_size());

        vec2 min_v(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
        vec2 max_v(std::max(p1.x, p2.x), std::max(p1.y, p2.y));

        return rectangle::from_min_max(min_v, max_v);
    }

    rectangle rectangle::from_min_max(const vec2& min, const vec2& max)
    {
        return { min, max };
    }

    rectangle rectangle::from_center_size(const vec2& center, const vec2& size)
    {
        return { center - size / 2.0f, center + size / 2.0f };
    }

    bool rectangle::contains(const vec2& o) const
    {
        return o.x >= min.x && o.x < max.x && o.y >= min.y && o.y < max.y;
    }

    rectangle rectangle::get_overlap(const rectangle& o) const
    {
        rectangle result;

        result.min.x = std::max(min.x, o.min.x);
        result.min.y = std::max(min.y, o.min.y);
        result.max.x = std::min(max.x, o.max.x);
        result.max.y = std::min(max.y, o.max.y);

        if (result.min.x < result.max.x && result.min.y < result.max.y)
            return result;
        else
            return { {0, 0}, {0, 0} }; // No overlap, return a rectangle with zero area
    }

    bool rectangle::has_overlap(const rectangle& o) const
    {
        auto over = get_overlap(o);

        return !(over.min.x == 0 && over.min.y == 0 && over.max.x == 0 && over.max.y == 0);
    }

    bool rectangle::contains(const rectangle& o) const
    {
        return o.min.x >= min.x && o.min.y >= min.y && o.max.x < max.x && o.max.y < max.y;
    }

    vec2 rectangle::center() const
    {
        return min + (max - min) / 2.0f;
    }

    vec2 rectangle::size() const
    {
        return max - min;
    }

    void nanovg_flush()
    {
        nvgEndFrame(vg);
        sg_reset_state_cache();
    }

    using fetch_id = int;

    struct fetch_user_data
    {
        fetch_callback callback;
        std::vector<char> buffer;
    };
    std::unordered_map<fetch_id, fetch_user_data> fetches;


    std::vector<char> pick_buffer(size_t file_size_hint)
    {
        if (file_size_hint == 0)
            file_size_hint = 100'000;

        return std::vector<char>(file_size_hint);
    }

    void fetch_response_callback(const sfetch_response_t* response)
    {
        fetch_id id = *(fetch_id*)response->user_data;

        fetch_user_data user_data = std::move(fetches[id]);
        fetches.erase(id);

        if (response->fetched)
        {
            size_t data_size = response->data.size;
            user_data.buffer.resize(data_size);

            user_data.callback(std::move(user_data.buffer));
        }
        else if (response->failed)
        {
            if (response->error_code == SFETCH_ERROR_BUFFER_TOO_SMALL)
            {
                size_t file_size_hint = 2 * user_data.buffer.size();
                fetch_file(response->path, user_data.callback, file_size_hint);
            }
            else
            {
                user_data.callback({});
            }
        }
    }

    void fetch_file(const char* file_path, fetch_callback callback, size_t file_size_hint)
    {
        static fetch_id fetch_id_counter = 1;

        fetch_user_data user_data;
        user_data.buffer = pick_buffer(file_size_hint);
        user_data.callback = callback;
        
        auto fetch_id = fetch_id_counter++;

        fetches[fetch_id] = std::move(user_data);

        auto& buffer = fetches[fetch_id].buffer;

        sfetch_request_t request{};
        request.path = file_path;
        request.callback = fetch_response_callback;
        request.user_data = { &fetch_id, sizeof(fetch_id) };
        request.buffer = { buffer.data(), buffer.size() };

        sfetch_send(&request);
    }
}

void frame_delta_update()
{
    auto now = std::chrono::high_resolution_clock::now();
    frame_delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_frame).count();
    start_frame = now;
}

void frame_update()
{
    frame_delta_update();

    sfetch_dowork();

    imgui::prepare_render();

    sg_begin_default_pass(&pass_action, (float)sapp_width(), (float)sapp_height());

    nvgBeginFrame(vg, (float)sapp_width(), (float)sapp_height(), 1.0f);

    nvgResetTransform(vg);
    apply_transform(transforms.back());

    update();

    nvgEndFrame(vg);

    sg_reset_state_cache();

    imgui::render();

    update_sg();

    sg_end_pass();

    sg_commit();

    events_end_frame();
}

void init()
{
#ifndef __EMSCRIPTEN__
    gladLoadGL();
#endif

    {
        sg_desc desc{};
        desc.context = sapp_sgcontext();
#ifdef _DEBUG
        desc.logger.func = slog_func;
#endif
        sg_setup(&desc);
    }

    stm_setup();

    imgui::setup(dump_font, sizeof(dump_font));

    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].clear_value = {0.0f, 0.0f, 0.0f, 0.0f};

#ifdef __EMSCRIPTEN__
    vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#else
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#endif

    {
        sfetch_desc_t desc{};
        sfetch_setup(&desc);
    }

    nvgCreateFontMem(vg, "default", dump_font, sizeof(dump_font), 0);

    setup_draw_sg();

    setup();
}

void cleanup()
{
    sg_shutdown();
}

void input(const sapp_event* event)
{
    if (!imgui::handle_event(event))
    {
        handle_event(event);
    }
}

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    start_application = start_frame = std::chrono::steady_clock::now();
    transforms.push_back(mat3::identity());

    sapp_desc desc{};
    desc.init_cb = init;
    desc.frame_cb = frame_update;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;
    desc.width = 800;
    desc.height = 600;
    desc.sample_count = 8; // antialiasing
    desc.alpha = true; // TODO test, what is doing this
    //desc.gl_force_gles2 = true;

#ifdef _DEBUG
    desc.win32_console_create = true;
    desc.win32_console_utf8 = true;
    desc.logger.func = slog_func;
#endif

    desc.window_title = "Framework";

    return desc;
}
