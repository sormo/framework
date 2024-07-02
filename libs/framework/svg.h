#pragma once
#include "nanosvg.h"
#include "framework.h"

namespace frame
{
    using svg_image = NSVGimage;

    svg_image* svg_parse(const char* input_xml);
    void svg_delete(svg_image* image);
    
    image svg_rasterize(svg_image* svg, uint32_t width, uint32_t height);

    frame::vec2 get_svg_image_size(svg_image* image);

    void draw_svg(svg_image* image, const vec2& position, frame::text_align align = frame::text_align::top_left);
    void draw_svg_ex(svg_image* image, const vec2& position, float radians, const vec2& scale, frame::text_align align = frame::text_align::top_left);
    void draw_svg_ex_size(svg_image* image, const vec2& world_position, float radians, const vec2& screen_size, frame::text_align align = frame::text_align::top_left); // specify exact size for the image instead fo scale
}
