#pragma once
#include "framework.h"

namespace frame
{
	void setup_instanced();

	using instanced_id = uint32_t;

	instanced_id create_instanced_rectangle();
	instanced_id create_instanced_circle(size_t count);

	size_t add_instanced(instanced_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);
	void remove_instanced(instanced_id id, size_t index);
	void update_instanced(instanced_id id, size_t index, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);

	void draw_instanced(instanced_id id);
}
