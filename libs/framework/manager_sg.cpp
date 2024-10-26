#include "manager_sg.h"
#include "sokol_glue.h"
#include "sokol_app.h"
#include "sokol_fontstash.h"

using namespace frame;

static image_t create_image_target(uint32_t width, uint32_t height, sg_pixel_format pixel_format, uint32_t samples)
{
    sg_image_desc image_desc = {};
    image_desc.pixel_format = pixel_format;
    image_desc.width = (int)width;
    image_desc.height = (int)height;
    image_desc.render_target = true;
    image_desc.sample_count = samples;
    image_desc.label = "image";

    return sg_make_image(&image_desc).id;
}

static void sgl_begin_pass()
{
    sgl_defaults();
    sgl_matrix_mode_projection();
    sgl_ortho(0.0f, sapp_widthf(), sapp_heightf(), 0.0f, -frame::max_depth, +frame::max_depth);
}

static void sgl_end_pass()
{
    sfons_flush(fons);
    sgl_draw();
}

static void sgl_end_pass(sgl_context& context)
{
    sfons_flush(fons);
    sgl_context_draw(context);
}

void manager_sg::initialize()
{
    set_default_pass_clear_color(col4::BLACK);

    default_target_desc.samples = sapp_sample_count();
    default_target_desc.pixel_format = (sg_pixel_format)sapp_color_format();
    default_target_desc.depth_stencil = (sg_pixel_format)sapp_depth_format() == SG_PIXELFORMAT_DEPTH_STENCIL; // if different is set, depth won't be working (or it will be crashing)
}

image_t manager_sg::create_render_target(uint32_t width, uint32_t height, const image_target_desc& desc)
{
    // create offscreen pass
    sg_attachments_desc attachments_desc = {};
    attachments_desc.colors[0].image = { create_image_target(width, height, desc.pixel_format, desc.samples) };
    if (desc.samples > 1)
        attachments_desc.resolves[0].image = { create_image_target(width, height, desc.pixel_format, 1) };
    if (desc.depth_stencil)
        attachments_desc.depth_stencil.image = { create_image_target(width, height, SG_PIXELFORMAT_DEPTH_STENCIL, desc.samples) };

    pass_data_t pass_data;
    pass_data.pass.attachments = sg_make_attachments(attachments_desc);

    sgl_context_desc_t context_desc = {};
    context_desc.color_format = desc.pixel_format;
    context_desc.depth_format = desc.depth_stencil ? SG_PIXELFORMAT_DEPTH_STENCIL : SG_PIXELFORMAT_NONE;
    context_desc.sample_count = desc.samples;

    pass_data.context = sgl_make_context(context_desc);
    pass_data.desc = desc;
    pass_data.width = width;
    pass_data.height = height;

    image_t color_image = desc.samples > 1 ? attachments_desc.resolves[0].image.id : attachments_desc.colors[0].image.id;
    
    passes.insert({ color_image, std::move(pass_data) });

    return color_image;
}

void manager_sg::begin_pass(image_t target)
{
    current_pass = &passes.at(target);

    current_pass->pass.action.colors[0].load_action = SG_LOADACTION_LOAD;

    sg_begin_pass(current_pass->pass);
    sgl_set_context(current_pass->context);
    sgl_begin_pass();

    is_in_pass = true;
}

void manager_sg::begin_pass_clear(image_t target, const col4& color)
{
    current_pass = &passes.at(target);

    current_pass->pass.action.colors[0].load_action = SG_LOADACTION_CLEAR;
    current_pass->pass.action.colors[0].clear_value = { color.data.r, color.data.g, color.data.b, color.data.a };

    sg_begin_pass(current_pass->pass);
    sgl_set_context(current_pass->context);
    sgl_begin_pass();

    is_in_pass = true;
}

void manager_sg::begin_default_pass()
{
    sg_pass pass = {};
    pass.action = default_pass_action;
    pass.swapchain = sglue_swapchain();

    current_pass = nullptr;

    sg_begin_pass(pass);
    sgl_set_context(sgl_default_context());
    sgl_begin_pass();

    is_in_pass = true;
}

void manager_sg::end_pass()
{
    is_in_pass = false;

    if (current_pass)
        sgl_end_pass(current_pass->context);
    else
        sgl_end_pass();
    sg_end_pass();
}

void manager_sg::set_default_pass_clear_color(const col4 & color)
{
    default_pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    default_pass_action.colors[0].clear_value = { color.data.r, color.data.g, color.data.b, color.data.a };
}

col4 manager_sg::get_default_pass_clear_color()
{
    return { default_pass_action.colors[0].clear_value.r, default_pass_action.colors[0].clear_value.g, default_pass_action.colors[0].clear_value.b, default_pass_action.colors[0].clear_value.a };
}

bool manager_sg::is_default_pass()
{
    return is_in_pass && current_pass == nullptr;
}

bool manager_sg::is_target(image_t target)
{
    return passes.count(target) != 0;
}

const image_target_desc& manager_sg::get_target_desc(image_t target)
{
    auto& pass = passes.at(target);

    return pass.desc;
}

const image_target_desc& manager_sg::get_default_target_desc()
{
    return default_target_desc;
}

std::pair<uint32_t, uint32_t> manager_sg::get_pass_size()
{
    if (is_default_pass())
        return { sapp_width(), sapp_height() };
    else
        return { current_pass->width, current_pass->height };
}

pipeline_t pipeline_manager_sg::make_pipeline(const sg_pipeline_desc& desc)
{
    pipeline_t result = pip_counter++;

    pipelines_data.insert({ result, { desc, nullptr, {} } });

    return result;
}

void pipeline_manager_sg::destroy_pipeline(pipeline_t pipeline)
{
    auto& pip_data = pipelines_data.at(pipeline);

    for (auto& [_, pip] : pip_data.pipelines)
        sg_destroy_pipeline(pip);

    pipelines_data.erase(pipeline);
}

void pipeline_manager_sg::apply_pipeline(pipeline_t pipeline)
{
    auto& pip_data = pipelines_data.at(pipeline);

    sg_apply_pipeline(*pip_data.pip_pass);
}

void pipeline_manager_sg::apply_pass(const image_target_desc& desc)
{
    for (auto& [_, pip_data] : pipelines_data)
    {
        if (pip_data.pipelines.count(desc) == 0)
        {
            pip_data.pipelines[desc] = make_pipeline(desc, pip_data.pip_template);
        }
        pip_data.pip_pass = &pip_data.pipelines[desc];
    }
}

sg_pipeline pipeline_manager_sg::make_pipeline(const image_target_desc& pass_data, const sg_pipeline_desc& pip_template)
{
    sg_pipeline_desc pip_desc = pip_template;

    pip_desc.sample_count = pass_data.samples;
    pip_desc.depth.write_enabled = pass_data.depth_stencil;
    pip_desc.depth.pixel_format = pass_data.depth_stencil ? SG_PIXELFORMAT_DEPTH_STENCIL : SG_PIXELFORMAT_NONE;
    pip_desc.colors[0].pixel_format = pass_data.pixel_format;

    return sg_make_pipeline(pip_desc);
}
