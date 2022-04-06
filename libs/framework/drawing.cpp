#include "framework.h"
#include <cmath>

void canvas_apply_text();

namespace frame
{
    namespace detail
    {
        // return number between 0 to 1 of current eighth
        float get_angle_eighth(float radians)
        {
            return std::fmodf(radians, NVG_PI / 4.0f) / (NVG_PI / 4.0f);
        }

        Direction get_start(float radians)
        {
            if (radians < 1.0f * NVG_PI / 4.0f)
                return Direction::Right;
            else if (radians < 2.0f * NVG_PI / 4.0f)
                return Direction::UpRight;
            else if (radians < 3.0f * NVG_PI / 4.0f)
                return Direction::Up;
            else if (radians < 4.0f * NVG_PI / 4.0f)
                return Direction::UpLeft;
            else if (radians < 5.0f * NVG_PI / 4.0f)
                return Direction::Left;
            else if (radians < 6.0f * NVG_PI / 4.0f)
                return Direction::DownLeft;
            else if (radians < 7.0f * NVG_PI / 4.0f)
                return Direction::Down;
            else if (radians < 8.0f * NVG_PI / 4.0f)
                return Direction::DownRight;
            return Direction::Right; // never happen
        }

        Direction get_end(float radians)
        {
            radians += NVG_PI / 4.0f;
            if (radians > 2.0f * NVG_PI)
                radians -= 2.0f * NVG_PI;

            return get_start(radians);
        }

        // return point on text rectangle which should match with line on point
        Point get_rect_point(Direction direction)
        {
            switch (direction)
            {
            case Direction::Right:
                return { -0.5f,  0.0f };
            case Direction::UpRight:
                return { -1.0f,  0.0f };
            case Direction::Up:
                return { -1.0f, -0.5f };
            case Direction::UpLeft:
                return { -1.0f, -1.0f };
            case Direction::Left:
                return { -0.5f, -1.0f };
            case Direction::DownLeft:
                return { 0.0f, -1.0f };
            case Direction::Down:
                return { 0.0f, -0.5f };
            case Direction::DownRight:
                return { 0.0f,  0.0f };
            }
            return {}; // never happen
        }
    }

    Point get_direction_vector(Direction direction)
    {
        switch (direction)
        {
        case Direction::Right:
            return { 1.0f, 0.0f };
        case Direction::UpRight:
            return { 0.5f, 0.5f };
        case Direction::Up:
            return { 0.0f, 1.0f };
        case Direction::UpLeft:
            return { -0.5f, 0.5f };
        case Direction::Left:
            return { -1.0f, 0.0f };
        case Direction::DownLeft:
            return { -0.5f, -0.5f };
        case Direction::Down:
            return { 0.0f, -1.0f };
        case Direction::DownRight:
            return { 0.5f, -0.5f };
        }
        return {}; // never happen
    }

    void draw_rectangle(const Point& position, float width, float height, const Color& color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());

        nvgBeginPath(vg);
        nvgMoveTo(vg, -hw, -hh);
        nvgLineTo(vg, -hw,  hh);
        nvgLineTo(vg,  hw,  hh);
        nvgLineTo(vg,  hw, -hh);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_rectangle_ex(const Point& position,
                           float radians,
                           float width,
                           float height,
                           const Color& fill_color,
                           const float outline_thickness,
                           const Color& outline_color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());
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

    void draw_rounded_rectangle(const Point& position, float width, float height, float radius, const Color& color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());

        nvgBeginPath(vg);
        nvgRoundedRect(vg, -hw, -hh, hw, hh, radius);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_rounded_rectangle_ex(const Point& position,
                                   float radians,
                                   float width,
                                   float height,
                                   float radius,
                                   const Color& fill_color,
                                   const float outline_thickness,
                                   const Color& outline_color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());
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

    void draw_circle(const Point& position, float radius, const Color& color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());

        nvgBeginPath(vg);
        nvgMoveTo(vg, -radius, 0.0f);
        nvgArcTo(vg, -radius, radius, 0.0f, radius, radius);
        nvgArcTo(vg, radius, radius, radius, 0.0f, radius);
        nvgArcTo(vg, radius, -radius, 0.0f, -radius, radius);
        nvgArcTo(vg, -radius, -radius, -radius, 0.0f, radius);

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_circle_ex(const Point& position,
                        float radians,
                        float radius,
                        const Color& fill_color,
                        const float outline_thickness,
                        const Color& outline_color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgMoveTo(vg, -radius, 0.0f);
        nvgArcTo(vg, -radius, radius, 0.0f, radius, radius);
        nvgArcTo(vg, radius, radius, radius, 0.0f, radius);
        nvgArcTo(vg, radius, -radius, 0.0f, -radius, radius);
        nvgArcTo(vg, -radius, -radius, -radius, 0.0f, radius);

        nvgFillColor(vg, fill_color.data);
        nvgFill(vg);

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_polygon(const Point& position, const Point* vertices, size_t count, const Color& color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());

        nvgBeginPath(vg);
        nvgMoveTo(vg, vertices[0].x(), vertices[0].y());
        for (size_t i = 1; i < count; i++)
            nvgLineTo(vg, vertices[i].x(), vertices[i].y());

        nvgFillColor(vg, color.data);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void draw_polygon_ex(const Point& position,
                         float radians,
                         const Point* vertices,
                         size_t count,
                         const Color& fill_color,
                         const float outline_thickness,
                         const Color& outline_color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x(), position.y());
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgMoveTo(vg, vertices[0].x(), vertices[0].y());
        for (size_t i = 1; i < count; i++)
            nvgLineTo(vg, vertices[i].x(), vertices[i].y());
        nvgClosePath(vg);

        nvgFillColor(vg, fill_color.data);
        nvgFill(vg);

        nvgStrokeWidth(vg, outline_thickness);
        nvgStrokeColor(vg, outline_color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_directed(const Point& from, const Point& to, const Color& color)
    {
        static const float arrowLength = 5.0f;
        static const float arrowAngle = nvgDegToRad(45.0f);
        static const float offset = std::sinf(arrowAngle) * arrowLength;

        auto vec = to - from;

        float angle = vec.Angle();
        float length = vec.Length();

        nvgSave(vg);

        nvgTranslate(vg, from.x(), from.y());
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

    void draw_line_directed_ex(const Point& from, const Point& to, float thickness, const Color& color)
    {
        static const float arrowLength = 5.0f;
        static const float arrowAngle = nvgDegToRad(45.0f);
        static const float offset = std::sinf(arrowAngle) * arrowLength;

        auto vec = to - from;

        float angle = vec.Angle();
        float length = vec.Length();

        nvgSave(vg);

        nvgTranslate(vg, from.x(), from.y());
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

    void draw_line_solid(const Point& from, const Point& to, const Color& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x(), from.y());
        nvgLineTo(vg, to.x(), to.y());

        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_solid_ex(const Point& from, const Point& to, float thickness, const Color& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x(), from.y());
        nvgLineTo(vg, to.x(), to.y());

        nvgStrokeWidth(vg, thickness);
        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_dashed(const Point& from, const Point& to, const Color& color)
    {
        static const float DashLength = 3.0f;

        float m = (to.y() - from.y()) / (to.x() - from.x());
        float b = to.y() - m * to.x();

        //nvgBeginPath(vg);
        //nvgStrokeColor(vg, color);

        nvgSave(vg);

        float x1 = from.x();
        while (x1 + DashLength < to.x())
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

    void draw_line_dashed_ex(const Point& from, const Point& to, float thickness, const Color& color)
    {
        static const float DashLength = 3.0f;

        float m = (to.y() - from.y()) / (to.x() - from.x());
        float b = to.y() - m * to.x();

        //nvgBeginPath(vg);
        //nvgStrokeColor(vg, color);

        nvgSave(vg);

        float x1 = from.x();
        while (x1 + DashLength < to.x())
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

    void draw_curve_quad(const Point& from, const Point& control, const Point& to, const Color& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x(), from.y());
        nvgQuadTo(vg, control.x(), control.y(), to.x(), to.y());

        nvgStrokeColor(vg, color.data);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_curve_quad_ex(const Point& from, const Point& control, const Point& to, float thickness, const Color& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x(), from.y());
        nvgQuadTo(vg, control.x(), control.y(), to.x(), to.y());

        nvgStrokeColor(vg, color.data);
        nvgStrokeWidth(vg, thickness);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void draw_line_text(const Point& from, const Point& to, const char* text, float position, float size, float offset, Side side, const Color& color)
    {
        Point lineVector = to - from;
        float lineAngle = lineVector.Angle();
        Point linePoint = from + lineVector * position;
        Point textSize = frame::text_dimensions(text, size);

        // apply offset
        linePoint = linePoint + lineVector.Perpendicular(side).Normalized() * offset;

        Point textStart = linePoint, textEnd = linePoint;
        if (side == Side::Left)
        {
            textStart = textStart + textSize * detail::get_rect_point(detail::get_start(lineAngle));
            textStart = textStart + textSize / 2.0f;
            textEnd = textEnd + textSize * detail::get_rect_point(detail::get_end(lineAngle));
            textEnd = textEnd + textSize / 2.0f;
        }
        else
        {
            textStart = textStart - textSize * detail::get_rect_point(detail::get_start(lineAngle));
            textStart = textStart - textSize / 2.0f;
            textEnd = textEnd - textSize * detail::get_rect_point(detail::get_end(lineAngle));
            textEnd = textEnd - textSize / 2.0f;
        }

        Point rectPosition = textStart.Interpolate(textEnd, detail::get_angle_eighth(lineAngle));
        //frame::rectangle(rectPosition, 0.0f, textSize.x(), textSize.y(), nvgRGBA(50, 80, 60, 120));
        frame::text(text, rectPosition - textSize / 2.0f, size, color);
    }

    float text_height(const char* text, float width, float size)
    {
        float bounds[4];
        nvgFontSize(vg, size);
        nvgTextBoxBounds(vg, 0.0f, 0.0f, width, text, nullptr, bounds);

        return bounds[3] - bounds[1];
    }

    float text_width(const char* text, float size)
    {
        float bounds[4];
        nvgFontSize(vg, size);
        nvgTextBoxBounds(vg, 0.0f, 0.0f, std::numeric_limits<float>::max(), text, nullptr, bounds);

        return bounds[2] - bounds[0];
    }

    Point text_dimensions(const char* text, float size)
    {
        float bounds[4];
        nvgFontSize(vg, size);
        nvgTextBoxBounds(vg, 0.0f, 0.0f, std::numeric_limits<float>::max(), text, nullptr, bounds);

        return { bounds[2] - bounds[0], bounds[3] - bounds[1] };
    }

    void text(const char* text, const Point& position, float width, float size, const Color& color)
    {
        nvgSave(vg);

        nvgResetTransform(vg);
        canvas_apply_text();

        float bounds[4];
        nvgFontSize(vg, size);
        nvgTextBoxBounds(vg, 0.0f, 0.0f, width, text, nullptr, bounds);

        float x = position.x() - bounds[0];
        float y = -bounds[1] - bounds[3] - position.y() - size;

        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        nvgFillColor(vg, color.data);
        nvgTextBox(vg, x, y, width, text, nullptr);

        nvgRestore(vg);
    }

    void text(const char* text, const Point& position, float size, const Color& color)
    {
        nvgSave(vg);

        nvgResetTransform(vg);
        canvas_apply_text();

        float bounds[4];
        nvgFontSize(vg, size);
        nvgTextBoxBounds(vg, 0.0f, 0.0f, std::numeric_limits<float>::max(), text, nullptr, bounds);

        float x = position.x() - bounds[0];
        float y = -bounds[1] - bounds[3] - position.y() - size;

        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        nvgFontSize(vg, size);
        nvgFillColor(vg, color.data);
        nvgTextBox(vg, x, y, std::numeric_limits<float>::max(), text, nullptr);

        nvgRestore(vg);
    }
}
