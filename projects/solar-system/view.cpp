#include "view.h"

using namespace frame;

struct
{
	std::function<vec2()> world_position;
	double scale = 1.0;

} state;

namespace view
{
	namespace impl
	{
		// create world transform from given view transform
		frame::mat3 get_transform_to_world(const mat3& view_transform)
		{
			auto translation = view_transform.get_translation() - state.world_position() * view_transform.get_scale() * state.scale;
			auto scale = view_transform.get_scale() * state.scale;

			return frame::translation(translation) * frame::scale(scale);
		}

		// create view transform from given world transform
		frame::mat3 get_transform_to_view(const mat3& world_transform)
		{
			auto translation = frame::get_screen_size() / 2.0f;
			auto scale = world_transform.get_scale();

			auto world_translation = world_transform.inverted().transform_point(frame::get_screen_size() / 2.0f) - state.world_position();

			frame::save_world_transform();
			frame::set_world_transform(frame::translation(translation) * frame::scale(scale));

			// TODO when I have time, this should be cleaned up
			frame::set_world_translation(frame::get_screen_size() / 2.0f, world_translation);

			auto result = frame::get_world_transform() * frame::scale({ 1.0 / state.scale, 1.0 / state.scale });;

			frame::restore_world_transform();

			return result;
		}
	}

	void set_view(const frame::vec2& world_position, double scale)
	{
		set_view([world_position]() { return world_position; }, scale);
	}

	void set_view(std::function<vec2()> world_position, double scale)
	{
		clear_view();

		state.scale = scale;
		state.world_position = world_position;

		frame::set_world_transform(impl::get_transform_to_view(frame::get_world_transform()));
	}

	void clear_view()
	{
		if (state.world_position)
		{
			frame::set_world_transform(impl::get_transform_to_world(frame::get_world_transform()));

			state.world_position = {};
			state.scale = 1.0;
		}
	}

	frame::mat3 get_world_transform()
	{
		if (state.world_position)
		{
			return impl::get_transform_to_world(frame::get_world_transform());
		}
		else
		{
			return frame::get_world_transform();
		}
	}

	frame::vec2 get_world_to_view(const frame::vec2& world_position)
	{
		auto screen_position = impl::get_transform_to_world(frame::get_world_transform()).transform_point(world_position);

		return frame::get_world_transform().inverted().transform_point(screen_position);
	}

	frame::vec2 get_view_to_world(const frame::vec2& view_position)
	{
		auto screen_position = frame::get_world_transform().inverted().transform_point(view_position);

		return impl::get_transform_to_world(frame::get_world_transform()).transform_point(screen_position);
	}

	frame::vec2 get_screen_to_world(const frame::vec2& screen_position)
	{
		if (state.world_position)
		{
			return impl::get_transform_to_world(frame::get_world_transform()).inverted().transform_point(screen_position);
		}
		else
		{
			return frame::get_world_transform().inverted().transform_point(screen_position);
		}
	}

	float get_pixel_to_world(float s)
	{
		return s / (state.scale * frame::get_world_scale().x);
	}

	float get_world_to_pixel(float s)
	{
		return s * state.scale * frame::get_world_scale().x;
	}

	float get_world_to_view(float s)
	{
		return s * state.scale;
	}

	double get_world_to_view(double s)
	{
		return s * state.scale;
	}

	double get_scale()
	{
		return state.scale;
	}
}
