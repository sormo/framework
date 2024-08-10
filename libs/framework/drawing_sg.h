#pragma once
#include "framework.h"
#include "sokol_gfx.h"
#include "HandmadeMath.h"

namespace frame
{
	using draw_buffer_id = uint32_t;
	constexpr draw_buffer_id draw_buffer_id_invalid = 0;

	enum class mesh_t
	{
		basic, // two float (x, y) per vertex
		depth  // three float (x, y, depth) per vertex
	};

	struct mesh
	{
		mesh(float* v, size_t vc, uint16_t* i = nullptr, size_t ic = 0, mesh_t t = mesh_t::basic) : vertices(v), vertices_count(vc), indices(i), indices_count(ic), type(t) {}

		float* vertices = nullptr;
		size_t vertices_count = 0;
		uint16_t* indices = nullptr;
		size_t indices_count = 0;

		mesh_t type = mesh_t::basic;
	};

	struct mesh_data
	{
		std::vector<float> vertices; // also vec2/vec3 can be
		std::vector<uint16_t> indices;

		mesh_t type = mesh_t::basic;

		operator mesh() { return { vertices.data(), vertices.size() / 2, indices.data(), indices.size(), type }; }
	};

	mesh_data create_mesh_rectangle();
	mesh_data create_mesh_circle(size_t count);
	mesh_data create_mesh_circle(size_t count, float depth);
	mesh_data create_mesh_circle_no_index(size_t count);

	// stride seems to be limited to [0,255] on webgl (which is unfortunate, on desktop opengl works fine larger stride)
	draw_buffer_id create_draw_buffer(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage, uint8_t stride_in_bytes = 0);
	draw_buffer_id create_draw_buffer_instanced(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage);
	draw_buffer_id create_draw_buffer_instanced(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage, size_t instances_count);

	size_t add_draw_instance(draw_buffer_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);
	size_t add_draw_instance(draw_buffer_id id, frame::vec3 position, float rotation, frame::vec2 size, frame::col4 color);
	size_t add_draw_instance(draw_buffer_id id, const frame::mat3& transform, frame::col4 color);
	void remove_draw_instance(draw_buffer_id id, size_t index);
	void update_draw_instance(draw_buffer_id id, size_t index, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);
	void update_draw_instance(draw_buffer_id id, size_t index, const frame::mat3& transform, frame::col4 color);
	void update_draw_instance(draw_buffer_id id, size_t index, const frame::vec2& position, const frame::col4& color);
	void update_draw_instance(draw_buffer_id id, size_t index, const frame::vec3& position, const frame::col4& color);

	void draw_buffer(draw_buffer_id id, frame::col4 color);
	void draw_buffer(draw_buffer_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color);
	void draw_buffer(draw_buffer_id id, frame::vec3 position, float rotation, frame::vec2 size, frame::col4 color);
	void draw_buffer(draw_buffer_id id, const frame::mat3& transform, frame::col4 color);
	void draw_buffer_instanced(draw_buffer_id id);
	void draw_buffer_instanced(draw_buffer_id id, size_t count);

	// optimization to avoid creating multiple projection/view matrices
	void draw_buffers(const std::vector<draw_buffer_id>& ids, const std::vector<frame::mat3>& transforms, const std::vector<frame::col4>& colors);
	void draw_buffers(const std::vector<draw_buffer_id>& ids, const std::vector<hmm_mat4>& transforms, const std::vector<frame::col4>& colors);

	// ---

	void update_buffer(draw_buffer_id id, mesh mesh); // TODO
	void remove_buffer(draw_buffer_id id); // TODO

	// TODO remove

	draw_buffer_id create_instanced_rectangle();
	draw_buffer_id create_instanced_circle(size_t count);

	// --- drawing with custom shader ---
	// if user creates custom shader he should have all control over bingings and pipeline, add just some helper functions
	hmm_mat4 create_world_mvp(frame::vec2 position, float rotation, frame::vec2 size);
	hmm_mat4 create_world_mvp(frame::vec3 position, float rotation, frame::vec2 size);
	hmm_mat4 create_world_mvp(const frame::mat3& transform);

	// TODO what about getting rid of mat3 ???
	// drawing with depth allows using vec3 for position
	hmm_mat4 create_hmm_transform(frame::vec2 position, float rotation, frame::vec2 size);
	hmm_mat4 create_hmm_transform(frame::vec3 position, float rotation, frame::vec2 size);
}
