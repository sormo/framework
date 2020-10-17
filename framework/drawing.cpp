#include "framework.h"
#include <cmath>

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

    void rectangle(const Point& position, float radians, float width, float height, const NVGcolor& color)
    {
        float hw = width / 2.0f, hh = height / 2.0f;

        nvgSave(vg);

        nvgTranslate(vg, position.x(), frame::canvas_height() - position.y());
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgMoveTo(vg, -hw, -hh);
        nvgLineTo(vg, -hw,  hh);
        nvgLineTo(vg,  hw,  hh);
        nvgLineTo(vg,  hw, -hh);

        nvgFillColor(vg, color);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void circle(const Point& position, float radians, float radius, const NVGcolor& color)
    {
        nvgSave(vg);

        nvgTranslate(vg, position.x(), frame::canvas_height() - position.y());
        nvgRotate(vg, radians);

        nvgBeginPath(vg);
        nvgMoveTo(vg, -radius, 0.0f);
        nvgArcTo(vg, -radius, radius, 0.0f, radius, radius);
        nvgArcTo(vg, radius, radius, radius, 0.0f, radius);
        nvgArcTo(vg, radius, -radius, 0.0f, -radius, radius);
        nvgArcTo(vg, -radius, -radius, -radius, 0.0f, radius);

        nvgFillColor(vg, color);
        nvgFill(vg);

        nvgRestore(vg);
    }

    void line_directed(const Point& from, const Point& to, const NVGcolor& color)
    {
        static const float arrowLength = 5.0f;
        static const float arrowAngle = nvgDegToRad(45.0f);
        static const float offset = std::sinf(arrowAngle) * arrowLength;

        auto vec = to - from;

        float angle = vec.Angle();
        float length = vec.Length();

        nvgSave(vg);

        nvgTranslate(vg, from.x(), frame::canvas_height() - from.y());
        nvgRotate(vg, -angle);

        nvgBeginPath(vg);
        nvgMoveTo(vg, 0.0f, 0.0f);
        nvgLineTo(vg, length, 0.0f);

        nvgMoveTo(vg, length - arrowLength, -offset);
        nvgLineTo(vg, length, 0.0f);
        nvgLineTo(vg, length - arrowLength, offset);

        nvgStrokeColor(vg, color);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void line_solid(const Point& from, const Point& to, const NVGcolor& color)
    {
        nvgSave(vg);

        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x(), frame::canvas_height() - from.y());
        nvgLineTo(vg, to.x(), frame::canvas_height() - to.y());

        nvgStrokeColor(vg, color);
        nvgStroke(vg);

        nvgRestore(vg);
    }

    void line_dashed(const Point& from, const Point& to, const NVGcolor& color)
    {
        static const float DashLength = 3.0f;

        float m = (to.y() - from.y()) / (to.x() - from.x());
        float b = to.y() - m * to.x();

        //nvgBeginPath(vg);
        //nvgStrokeColor(vg, color);

        float x1 = from.x();
        while (x1 + DashLength < to.x())
        {
            float y1 = m * x1 + b;

            float x2 = x1 + DashLength;
            float y2 = m * x2 + b;

            nvgBeginPath(vg);
            nvgStrokeColor(vg, color);

            nvgMoveTo(vg, x1, frame::canvas_height() - y1);
            nvgLineTo(vg, x2, frame::canvas_height() - y2);
            nvgStroke(vg);

            x1 = x2 + DashLength;
        }
    }

    void curve_quad(const Point& from, const Point& control, const Point& to, const NVGcolor& color)
    {
        nvgBeginPath(vg);

        nvgMoveTo(vg, from.x(), frame::canvas_height() - from.y());
        nvgQuadTo(vg, control.x(), frame::canvas_height() - control.y(), to.x(), frame::canvas_height() - to.y());

        nvgStrokeColor(vg, color);
        nvgStroke(vg);
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

    void text(const char* text, const Point& position, float width, float size, const NVGcolor& color)
    {
        nvgSave(vg);

        float bounds[4];
        nvgFontSize(vg, size);
        nvgTextBoxBounds(vg, 0.0f, 0.0f, width, text, nullptr, bounds);

        float x = position.x() - bounds[0];
        float y = frame::canvas_height() - bounds[1] - bounds[3] - position.y() - size;

        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        nvgFillColor(vg, color);
        nvgTextBox(vg, x, y, width, text, nullptr);

        nvgRestore(vg);
    }

    void text(const char* text, const Point& position, float size, const NVGcolor& color)
    {
        nvgSave(vg);

        float bounds[4];
        nvgFontSize(vg, size);
        nvgTextBoxBounds(vg, 0.0f, 0.0f, std::numeric_limits<float>::max(), text, nullptr, bounds);

        float x = position.x() - bounds[0];
        float y = frame::canvas_height() - bounds[1] - bounds[3] - position.y() - size;

        nvgTextAlign(vg, NVG_ALIGN_MIDDLE | NVG_ALIGN_LEFT);
        nvgFontSize(vg, size);
        nvgFillColor(vg, color);
        nvgTextBox(vg, x, y, std::numeric_limits<float>::max(), text, nullptr);

        nvgRestore(vg);
    }

    void line_text(const Point& from, const Point& to, const char* text, float position, float size, float offset, Side side, const NVGcolor& color)
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
}
