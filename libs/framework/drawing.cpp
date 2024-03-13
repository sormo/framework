#include "framework.h"
#include <cmath>
#include <vector>

namespace frame
{
    void draw_rectangle(const vec2& position, float width, float height, const col4& color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);

        nvgBeginPath(vg);
        nvgMoveTo(vg, -hw, -hh);
        nvgLineTo(vg, -hw,  hh);
        nvgLineTo(vg,  hw,  hh);
        nvgLineTo(vg,  hw, -hh);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_rectangle_ex(const vec2& position,
                           float radians,
                           float width,
                           float height,
                           const col4& fill_color,
                           const float outline_thickness,
                           const col4& outline_color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgMoveTo(vg, -hw, -hh);
        nvgLineTo(vg, -hw, hh);
        nvgLineTo(vg, hw, hh);
        nvgLineTo(vg, hw, -hh);
        nvgClosePath(vg);

        nvgFillColor(vg, fill_color.data);
        nvgFill(vg);

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_rounded_rectangle(const vec2& position, float width, float height, float radius, const col4& color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, -hw, -hh, hw, hh, radius);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_rounded_rectangle_ex(const vec2& position,
                                   float radians,
                                   float width,
                                   float height,
                                   float radius,
                                   const col4& fill_color,
                                   const float outline_thickness,
                                   const col4& outline_color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgRoundedRect(vg, -hw, -hh, hw, hh, radius);

        nvgFillColor(vg, fill_color.data);
        nvgFill(vg);

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_circle(const vec2& position, float radius, const col4& color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);

        nvgBeginPath(vg);
        //nvgMoveTo(vg, -radius, 0.0f);
        //nvgArcTo(vg, -radius, radius, 0.0f, radius, radius);
        //nvgArcTo(vg, radius, radius, radius, 0.0f, radius);
        //nvgArcTo(vg, radius, -radius, 0.0f, -radius, radius);
        //nvgArcTo(vg, -radius, -radius, -radius, 0.0f, radius);
        nvgCircle(vg, 0.0f, 0.0f, radius);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_circle_ex(const vec2& position,
                        float radians,
                        float radius,
                        const col4& fill_color,
                        const float outline_thickness,
                        const col4& outline_color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        //nvgMoveTo(vg, -radius, 0.0f);
        //nvgArcTo(vg, -radius, radius, 0.0f, radius, radius);
        //nvgArcTo(vg, radius, radius, radius, 0.0f, radius);
        //nvgArcTo(vg, radius, -radius, 0.0f, -radius, radius);
        //nvgArcTo(vg, -radius, -radius, -radius, 0.0f, radius);
        nvgCircle(vg, 0.0f, 0.0f, radius);

        nvgFillColor(vg, fill_color.data);
        nvgFill(vg);

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_ellipse(const vec2& position, float major, float minor, const col4& color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);

        nvgBeginPath(vg);
        nvgEllipse(vg, 0.0f, 0.0f, major, minor);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_ellipse_ex(const vec2& position,
                         float radians,
                         float major,
                         float minor,
                         const col4& fill_color,
                         float outline_thickness,
                         const col4& outline_color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgEllipse(vg, 0.0f, 0.0f, major, minor);

        nvgFillColor(vg, fill_color.data);
        nvgFill(vg);

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_hyperbola(const vec2& position,
                        float radians,
                        float major,
                        float minor,
                        float outline_thickness,
                        const col4& outline_color)
    {
        static const float min_value = -10.0f;
        static const float max_value =  10.0f;
        static const float step_value = 0.1f;

        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);
        nvgRotate(vg, radians);

        nvgBeginPath(vg);

        vec2 last_point(major * cosh(min_value), minor * sinh(min_value));

        nvgMoveTo(vg, last_point.x, last_point.y);

        for (float theta = min_value + step_value; theta < max_value; theta += step_value)
        {
            vec2 new_point(major * cosh(theta), minor * sinh(theta));

            nvgQuadTo(vg, last_point.x, last_point.y, new_point.x, new_point.y);

            last_point = std::move(new_point);
        }

        // generate two halfs of hyperbola
        //nvgMoveTo(vg, -major * cosh(min_value), -minor * sinh(min_value));

        //for (float theta = min_value + step_value; theta < max_value; theta += step_value)
        //{
        //    vec2 new_point(-major * cosh(theta), -minor * sinh(theta));

        //    nvgQuadTo(vg, last_point.x, last_point.y, new_point.x, new_point.y);

        //    last_point = std::move(new_point);
        //}

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_polygon(const vec2& position, const vec2* vertices, size_t count, const col4& color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);

        nvgBeginPath(vg);
        nvgMoveTo(vg, vertices[0].x, vertices[0].y);
        for (size_t i = 1; i < count; i++)
            nvgLineTo(vg, vertices[i].x, vertices[i].y);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_polygon_ex(const vec2& position,
                         float radians,
                         const vec2* vertices,
                         size_t count,
                         const col4& fill_color,
                         const float outline_thickness,
                         const col4& outline_color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x, position.y);
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgMoveTo(vg, vertices[0].x, vertices[0].y);
        for (size_t i = 1; i < count; i++)
            nvgLineTo(vg, vertices[i].x, vertices[i].y);
        nvgClosePath(vg);

        nvgFillColor(vg, fill_color.data);
        nvgFill(vg);

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_directed(const vec2& from, const vec2& to, const col4& color)
    {
        static const float arrowLength = 5.0f;
        static const float arrowAngle = nvgDegToRad(45.0f);
        static const float offset = std::sinf(arrowAngle) * arrowLength;

        auto vec = to - from;

        float angle = vec.angle();
        float length = vec.length();

        nvgSave(vg);

        nvgTranslate(vg, from.x, from.y);
        nvgRotate(vg, angle);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0.0f, 0.0f);
        nvgLineTo(vg, length, 0.0f);

        nvgMoveTo(vg, length - arrowLength, -offset);
        nvgLineTo(vg, length, 0.0f);
        nvgLineTo(vg, length - arrowLength, offset);

        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_directed_ex(const vec2& from, const vec2& to, float thickness, const col4& color)
    {
        static const float arrowLength = 5.0f;
        static const float arrowAngle = nvgDegToRad(45.0f);
        static const float offset = std::sinf(arrowAngle) * arrowLength;

        auto vec = to - from;

        float angle = vec.angle();
        float length = vec.length();

        nvgSave(vg);

        nvgTranslate(vg, from.x, from.y);
        nvgRotate(vg, angle);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0.0f, 0.0f);
        nvgLineTo(vg, length, 0.0f);

        nvgMoveTo(vg, length - arrowLength, -offset);
        nvgLineTo(vg, length, 0.0f);
        nvgLineTo(vg, length - arrowLength, offset);

        nvgStrokeWidth(vg, thickness);
        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_solid(const vec2& from, const vec2& to, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x, from.y);
        nvgLineTo(vg, to.x, to.y);

        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_solid_ex(const vec2& from, const vec2& to, float thickness, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x, from.y);
        nvgLineTo(vg, to.x, to.y);

        nvgStrokeWidth(vg, thickness);
        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_dashed(const vec2& from, const vec2& to, const col4& color)
    {
        static const float DashLength = 3.0f;

        float m = (to.y - from.y) / (to.x - from.x);
        float b = to.y - m * to.x;

        //nvgBeginPath(vg);
        //nvgStrokeColor(vg, color);

        nvgSave(vg);

        float x1 = from.x;
        while (x1 + DashLength < to.x)
        {
            float y1 = m * x1 + b;

            float x2 = x1 + DashLength;
            float y2 = m * x2 + b;

            nvgBeginPath(vg);
            nvgStrokeColor(vg, color.data);

            nvgMoveTo(vg, x1, y1);
            nvgLineTo(vg, x2, y2);
            nvgStroke(vg);

            x1 = x2 + DashLength;
        }

        nvgRestore(vg);
    }

    void draw_line_dashed_ex(const vec2& from, const vec2& to, float thickness, const col4& color)
    {
        static const float DashLength = 3.0f;

        float m = (to.y - from.y) / (to.x - from.x);
        float b = to.y - m * to.x;

        //nvgBeginPath(vg);
        //nvgStrokeColor(vg, color);

        nvgSave(vg);

        float x1 = from.x;
        while (x1 + DashLength < to.x)
        {
            float y1 = m * x1 + b;

            float x2 = x1 + DashLength;
            float y2 = m * x2 + b;

            nvgBeginPath(vg);
            nvgStrokeColor(vg, color.data);

            nvgMoveTo(vg, x1, y1);
            nvgLineTo(vg, x2, y2);
            nvgStrokeWidth(vg, thickness);
            nvgStroke(vg);

            x1 = x2 + DashLength;
        }

        nvgRestore(vg);
    }

    void draw_quad_bezier(const vec2& from, const vec2& control, const vec2& to, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x, from.y);
        nvgQuadTo(vg, control.x, control.y, to.x, to.y);

        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_quad_bezier_ex(const vec2& from, const vec2& control, const vec2& to, float thickness, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x, from.y);
        nvgQuadTo(vg, control.x, control.y, to.x, to.y);

        nvgStrokeColor(vg, color.data);
        nvgStrokeWidth(vg, thickness);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_quad_bezier_polyline(const std::vector<vec2>& points, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, points[0].x, points[0].y);
        for (size_t i = 2; i < points.size(); i++)
            nvgQuadTo(vg, points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);

        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_quad_bezier_polyline_ex(const std::vector<vec2>& points, float thickness, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, points[0].x, points[0].y);
        for (size_t i = 2; i < points.size(); i++)
            nvgQuadTo(vg, points[i - 1].x, points[i - 1].y, points[i].x, points[i].y);

        nvgStrokeColor(vg, color.data);
        nvgStrokeWidth(vg, thickness);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_polyline(const std::vector<vec2>& points, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, points[0].x, points[0].y);
        for (size_t i = 1; i < points.size(); i++)
            nvgLineTo(vg, points[i].x, points[i].y);

        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_polyline_ex(const std::vector<vec2>& points, float thickness, const col4& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, points[0].x, points[0].y);
        for (size_t i = 1; i < points.size(); i++)
            nvgLineTo(vg, points[i].x, points[i].y);

        nvgStrokeColor(vg, color.data);
        nvgStrokeWidth(vg, thickness);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    int map_to_nvg_align(text_align align)
    {
        switch (align)
        {
        case text_align::top_left: return NVG_ALIGN_TOP | NVG_ALIGN_LEFT;
        case text_align::top_middle: return NVG_ALIGN_TOP | NVG_ALIGN_MIDDLE;
        case text_align::top_right: return NVG_ALIGN_TOP | NVG_ALIGN_RIGHT;
        case text_align::middle_left: return NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT;
        case text_align::middle_middle: return NVG_ALIGN_MIDDLE | NVG_ALIGN_MIDDLE;
        case text_align::middle_right: return NVG_ALIGN_MIDDLE | NVG_ALIGN_RIGHT;
        case text_align::bottom_left: return NVG_ALIGN_BOTTOM | NVG_ALIGN_LEFT;
        case text_align::bottom_midle: return NVG_ALIGN_BOTTOM | NVG_ALIGN_MIDDLE;
        case text_align::bottom_right: return NVG_ALIGN_BOTTOM | NVG_ALIGN_RIGHT;
        }
        return -1;
    }

    vec2 get_align_offset_factor(text_align align)
    {
        switch (align)
        {
        case text_align::top_left: return { -0.5f, -0.5f };
        case text_align::top_middle: return { 0.0f, -0.5f };
        case text_align::top_right: return { 0.5f, -0.5f };
        case text_align::middle_left: return { -0.5f, 0.0f };
        case text_align::middle_middle: return { 0.0f, 0.0f };
        case text_align::middle_right: return { 0.5f, 0.0f };
        case text_align::bottom_left: return { -0.5f, 0.5f };
        case text_align::bottom_midle: return { 0.0f, 0.5f };
        case text_align::bottom_right: return { 0.5f, 0.5f };
        }
        return {};
    }

    rectangle get_text_rectangle(const char* text, const vec2& position, float size, text_align align)
    {
        float bounds[4] = {};

        nvgFontSize(vg, size);
        nvgTextAlign(vg, map_to_nvg_align(align));

        nvgTextBoxBounds(vg, 0.0f, 0.0f, std::numeric_limits<float>::max(), text, nullptr, bounds);

        vec2 min = vec2{ bounds[0], bounds[1] } / get_world_transform().get_scale();
        vec2 max = vec2{ bounds[2], bounds[3] } / get_world_transform().get_scale();
        vec2 rect_size = (max - min);

        vec2 align_offset = rect_size * 0.5f;

        return rectangle::from_center_size(min + position + align_offset, rect_size.abs());
    }

    void set_text_transform(const vec2& position)
    {
        // TODO
        auto t = get_world_transform().get_translation();
        auto s = get_world_transform().get_scale();

        nvgResetTransform(vg);
        nvgTransform(vg, 1.0f, 0.0f, 0.0f, 1.0f, t.x + position.x * s.x, t.y + position.y * s.y);
    }

    void draw_text(const char* text, const vec2& position, float size, const col4& color, text_align align)
    {
        nvgSave(vg);

        set_text_transform(position);

        nvgFontSize(vg, size);
        nvgFillColor(vg, color.data);
        nvgTextAlign(vg, map_to_nvg_align(align));

        //nvgText(vg, 0.0f, 0.0f, text, nullptr);
        // to support newline we use this
        nvgTextBox(vg, 0.0f, 0.0f, std::numeric_limits<float>::max(), text, nullptr);

        nvgRestore(vg);
    }
}
