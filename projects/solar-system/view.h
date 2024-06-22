#pragma once
#include <framework.h>
#include <functional>

namespace view
{
	void set_view(const frame::vec2& world_position, double scale);
	void set_view(std::function<frame::vec2()> world_position, double scale);
	void clear_view();

	frame::mat3 get_world_transform();

	frame::vec2 get_world_to_view(const frame::vec2& world_position);
	frame::vec2 get_view_to_world(const frame::vec2& view_position);

	frame::vec2 get_screen_to_world(const frame::vec2& screen_position);
	frame::vec2 get_screen_to_world_vector(const frame::vec2& screen_position);

	// TODO verify this, there is something wrong, see the TODO in trajectory_resolutions::draw
	float get_pixel_to_world(float s);
	float get_pixel_to_view(float s);
	float get_world_to_pixel(float s);
	double get_world_to_pixel(double s);
	float get_world_to_view(float s);
	double get_world_to_view(double s);

	// should not be used
	double get_scale();
}
