#pragma once
#include "framework.h"
#include "sokol_gfx.h"

namespace frame
{
	void setup_draw_sg();

	using draw_buffer_id = uint32_t;

	struct mesh
	{
		float* vertices = nullptr;
		size_t vertices_count = 0;
		uint16_t* indices = nullptr;
		size_t indices_count = 0;
	};

	struct mesh_data
	{
		std::vector<float> vertices; // also vec2 can be
		std::vector<uint16_t> indices;

		operator mesh() { return { vertices.data(), vertices.size(), indices.data(), indices.size() }; }
	};

	mesh_data create_mesh_rectangle();
	mesh_data create_mesh_circle(size_t count);

	draw_buffer_id create_draw_buffer_instanced(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage, size_t max_count);

	draw_buffer_id create_instanced_rectangle();
	draw_buffer_id create_instanced_circle(size_t count);

	size_t add_draw_instance(draw_buffer_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);
	void remove_draw_instance(draw_buffer_id id, size_t index);
	void update_draw_instance(draw_buffer_id id, size_t index, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);

	void draw_buffer_instanced(draw_buffer_id id);

	draw_buffer_id create_draw_buffer(const char* name, mesh mesh, sg_primitive_type type);

	void draw_buffer(draw_buffer_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);
}
