#pragma once

namespace commons
{
	const double MIN_ZOOMED_SIZE = 0.1;
	const double DRAW_SIZE_FACTOR = 200.0;
	const double DRAW_VELOCITY_FACTOR = 13.0;

    frame::vec2 draw_cast(const frame::vec3d& p)
    {
        return { (float)(p.x * DRAW_SIZE_FACTOR), (float)(p.y * DRAW_SIZE_FACTOR) };
    }

    float scale_independent(float s)
    {
        return s / frame::get_world_scale().x;
    }

    std::vector<frame::vec2> draw_cast(const std::vector<frame::vec3d>& data)
    {
        std::vector<frame::vec2> r;
        r.reserve(data.size());
        for (const auto& o : data)
            r.push_back(draw_cast(o));
        return r;
    }
}
