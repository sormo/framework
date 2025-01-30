#ifdef __EMSCRIPTEN__
#define NANOVG_GLES3_IMPLEMENTATION
//#define NANOVG_GLES2_IMPLEMENTATION
#define SOKOL_GLES3
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif __ANDROID_API__
#define NANOVG_GLES3_IMPLEMENTATION
#define SOKOL_GLES3
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#define SOKOL_GLCORE
#define NOMINMAX
#include <Windows.h>
#define NANOVG_GL3_IMPLEMENTATION
//#define NANOVG_GLES3_IMPLEMENTATION
#include <glad/glad.h>
#define SOKOL_WIN32_NO_GL_LOADER
#endif

#include "framework.h"
#include "manager_sg.h"
#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_fetch.h"
#include "sokol_gl.h"
#include "sokol_time.h"
#include "sokol_log.h"
#include "imgui.h"
#undef SOKOL_IMPL

#define FONTSTASH_IMPLEMENTATION
#define FONTSTASH_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#include <fontstash.h>
#undef FONTSTASH_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#undef FONTSTASH_IMPLEMENTATION

#define SOKOL_FONTSTASH_IMPL
#include "sokol_fontstash.h"
#undef SOKOL_FONTSTASH_IMPL

#include "imgui_impl.h"

#include "nanovg.h"
#include "nanovg_gl.h"

#include "events.h"
#include "imgui_font.h"
#include <chrono>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>

using namespace frame;

NVGcontext* vg;
FONScontext* fons;
manager_sg pass_manager;
pipeline_manager_sg pip_manager;

std::chrono::time_point<std::chrono::high_resolution_clock> start_application;
std::chrono::time_point<std::chrono::high_resolution_clock> start_frame;
float frame_delta = 0.0f;

// TODO store also lazily inversion
std::vector<mat4> transforms;
//std::vector<mat3> transforms;

void apply_transform(const mat4& m)
{
    //   [a c e]
    //   [b d f]
    //   [0 0 1]
    //               a                      b                      c                      d                      e                      f
    nvgTransform(vg, m.data.Elements[0][0], m.data.Elements[0][1], m.data.Elements[1][0], m.data.Elements[1][1], m.data.Elements[3][0], m.data.Elements[3][1]);
}

void apply_transform(const mat3& m)
{
    nvgTransform(vg, m.data[0], m.data[3], m.data[1], m.data[4], m.data[2], m.data[5]);
}

namespace frame
{
    void setup_draw_sg();
    void setup_draw();

    col4 rgb(uint8_t r, uint8_t g, uint8_t b)
    {
        return col4::RGB(r, g, b);
    }

    col4 rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
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

    double rad_to_deg(double rad)
    {
        return rad * 57.2958;
    }

    void set_screen_background(const col4& color)
    {
        pass_manager.set_default_pass_clear_color(color);
    }

    col4 get_screen_background()
    {
        return pass_manager.get_default_pass_clear_color();
    }

    vec2 get_screen_size()
    {
        return { sapp_widthf(), sapp_heightf() };
    }

    float get_delta_time()
    {
        return frame_delta;
    }

    float get_time()
    {
        auto now = std::chrono::high_resolution_clock::now();
        return (float)std::chrono::duration_cast<std::chrono::milliseconds>(now - start_application).count();
    }

    mat4 translation(const vec2& translation)
    {
        return mat4::translation(translation);
    }

    mat4 translation(const vec3& translation)
    {
        return mat4::translation(translation);
    }


    mat4 rotation(float rotation)
    {
        return mat4::rotation(rotation);
    }

    mat4 rotation(const vec2& center, float rotation)
    {
        return mat4::rotation(center, rotation);
    }

    mat4 scale(const vec2& scale)
    {
        return mat4::scaling(scale);
    }

    mat4 scale(const vec3& scale)
    {
        return mat4::scaling(scale);
    }

    mat4 identity()
    {
        return mat4::identity();
    }

    void set_world_transform(const mat4& transform)
    {
        if (transforms.size() == 1)
            transforms.push_back(transform);
        else
            transforms.back() = transform;

        nvgResetTransform(vg);
        apply_transform(transform);
    }

    void set_world_transform_multiply(const mat4& transform)
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

    const mat4& get_world_transform()
    {
        return transforms.back();
    }

    vec2 get_screen_to_world(const vec2& screen_position)
    {
        return transforms.back().inverted().transform_point(screen_position);
    }

    vec2 get_screen_to_world_vector(const vec2& screen_position)
    {
        return transforms.back().inverted().transform_vector(screen_position);
    }

    vec2 get_world_to_screen(const vec2& world_position)
    {
        return transforms.back().transform_point(world_position);
    }

    vec2 get_world_to_screen_vector(const vec2& world_position)
    {
        return transforms.back().transform_vector(world_position);
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

    void set_world_translation(const vec3& translation)
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

        float c = screen_point.x - M.Elements[0][0] * world_point.x - M.Elements[1][0] * world_point.y;
        float f = screen_point.y - M.Elements[0][1] * world_point.x - M.Elements[1][1] * world_point.y;
		// mat3 :
        //float c = screen_point.x - M[0] * world_point.x - M[1] * world_point.y;
        //float f = screen_point.y - M[3] * world_point.x - M[4] * world_point.y;

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

    void set_world_scale(const vec3& scale)
    {
        transforms.back().set_scale(scale);
        set_world_transform(transforms.back());
    }


    void set_world_scale(const vec2& scale, const vec2& stationary_world_point)
    {
        mat4 new_transform = mat4::scaling(scale);
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
            float c = s.x - new_transform.data.Elements[0][0] * w.x - new_transform.data.Elements[1][0] * w.y;
            float f = s.y - new_transform.data.Elements[0][1] * w.x - new_transform.data.Elements[1][1] * w.y;
			// mat3:
            //float c = s.x - new_transform.data[0] * w.x - new_transform.data[1] * w.y;
            //float f = s.y - new_transform.data[3] * w.x - new_transform.data[4] * w.y;

            new_transform.set_translation({ c, f });
        }
        set_world_transform(new_transform);
    }

    void set_world_scale(const vec3& scale, const vec3& stationary_world_point)
    {
        mat4 new_transform = mat4::scaling(scale);
        {
            vec3 s = transforms.back().transform_point(stationary_world_point);
            const vec3& w = stationary_world_point;
            float c = s.x - new_transform.data.Elements[0][0] * w.x - new_transform.data.Elements[1][0] * w.y;
            float f = s.y - new_transform.data.Elements[0][1] * w.x - new_transform.data.Elements[0][1] * w.y;
            //float i = s.z - new_transform.data[6] * w.x - new_transform.data[7] * w.y;

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
        return !(max.x < o.min.x || min.x > o.max.x || max.y < o.min.y || min.y > o.max.y);
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

    void fetch_files(const std::vector<std::string>& files, std::function<void(std::map<std::string, std::vector<char>> file_data)> finished_callback)
    {
        std::shared_ptr<std::map<std::string, std::vector<char>>> file_data = std::make_shared<std::map<std::string, std::vector<char>>>();
        for (const auto& file : files)
        {
            frame::fetch_file(file.c_str(), [files, file, finished_callback, file_data](std::vector<char> data)
            {
                file_data->insert({ file, std::move(data) });
                if (file_data->size() == files.size())
                    finished_callback(std::move(*file_data));
            });
        }
    }

    image_t create_image_target(uint32_t width, uint32_t height, const image_target_desc& desc)
    {
        return pass_manager.create_render_target(width, height, desc);
    }

    void begin_pass(image_t target)
    {
        pass_manager.begin_pass(target);
        pip_manager.apply_pass(pass_manager.get_target_desc(target));
    }

    void begin_pass_clear(image_t target, const col4& color)
    {
        pass_manager.begin_pass_clear(target, color);
        pip_manager.apply_pass(pass_manager.get_target_desc(target));
    }

    void begin_default_pass()
    {
        pass_manager.begin_default_pass();
        pip_manager.apply_pass(pass_manager.get_default_target_desc());
    }

    void end_pass()
    {
        if (pass_manager.is_default_pass())
        {
            // render imgui at the end of default pass
            imgui::render();
        }

        pass_manager.end_pass();
    }

    const image_target_desc& get_image_target_desc(image_t image)
    {
        return pass_manager.get_target_desc(image);
    }

    bool is_image_target(image_t image)
    {
        return pass_manager.is_target(image);
    }

    bool image_target_desc::operator==(const image_target_desc& b) const
    {
        return depth_stencil == b.depth_stencil && pixel_format == b.pixel_format && samples == b.samples;
    }
}

void frame_delta_update()
{
    auto now = std::chrono::high_resolution_clock::now();
    frame_delta = (float)std::chrono::duration_cast<std::chrono::milliseconds>(now - start_frame).count();
    start_frame = now;
}

void frame_update()
{
    frame_delta_update();

    sfetch_dowork();

    imgui::prepare_render();

    nvgBeginFrame(vg, sapp_widthf(), sapp_heightf(), 1.0f);
    
    nvgResetTransform(vg);
    apply_transform(transforms.back());

    update();

    nvgEndFrame(vg);

    sg_reset_state_cache();

    sg_commit();

    events_end_frame();
}

void setup_sgl()
{
    sgl_desc_t desc = {};
    desc.logger.func = slog_func;

    sgl_setup(&desc);
}

// taken from sokol fontshath example
// round to next power of 2 (see bit-twiddling-hacks)
static int round_pow2(float v)
{
    uint32_t vi = ((uint32_t)v) - 1;
    for (uint32_t i = 0; i < 5; i++) {
        vi |= (vi >> (1 << i));
    }
    return (int)(vi + 1);
}

void setup_fontstash()
{
    const int atlas_dim = round_pow2(512.0f * sapp_dpi_scale());

    sfons_desc_t fons_desc = {};
    fons_desc.width = atlas_dim;
    fons_desc.height = atlas_dim;

    fons = sfons_create(&fons_desc);
}

void init()
{
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID_API__)
    gladLoadGL();
#endif

    {
        sg_desc desc{};
        desc.environment = sglue_environment();
#ifdef _DEBUG
        desc.logger.func = slog_func;
#endif
        sg_setup(&desc);
    }

    stm_setup();

    imgui::setup(dump_font, sizeof(dump_font));

    set_screen_background(frame::col4::RGBf(0.0f, 0.0f, 0.0f));

#if defined(__EMSCRIPTEN__) || defined(__ANDROID_API__)
    vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
	//vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#else
    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    //vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif

    {
        sfetch_desc_t desc{};
        sfetch_setup(&desc);
    }

    pass_manager.initialize();

    nvgCreateFontMem(vg, "default", dump_font, sizeof(dump_font), 0);

    setup_draw();
    setup_draw_sg();

    setup_sgl();
    
    setup_fontstash();

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
    transforms.push_back(mat4::identity());

    sapp_desc desc{};
    desc.init_cb = init;
    desc.frame_cb = frame_update;
    desc.cleanup_cb = cleanup;
    desc.event_cb = input;
    desc.width = 800;
    desc.height = 600;
    desc.sample_count = 4; // antialiasing
    desc.alpha = false; // TODO test, what is doing this
    desc.html5_premultiplied_alpha = false;
    //desc.html5_preserve_drawing_buffer = false;
    //desc.gl_force_gles2 = true;

#ifdef _DEBUG
    desc.win32_console_create = true;
    desc.win32_console_utf8 = true;
    desc.logger.func = slog_func;
#endif

    desc.window_title = "Framework";

    return desc;
}
