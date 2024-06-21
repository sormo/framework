#include "drawing_sg.h"
#include "utils.h"
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include "basic_instanced.glsl.h"
#include "basic.glsl.h"
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

namespace frame
{
	///////////////////////////////////////////
	/// BUFFERS ///////////////////////////////
	///////////////////////////////////////////

	class buffer
	{
		sg_buffer buffer_id = {};
		sg_usage usage = sg_usage::_SG_USAGE_DEFAULT;
		sg_buffer_type type = sg_buffer_type::_SG_BUFFERTYPE_DEFAULT;

		struct buffer_range
		{
			int offset;
			size_t size;
		};
		std::vector<buffer_range> ranges;

		std::vector<char> buffer_data;

		bool is_appended = false; // TODO can't update buffer while appending to it in the same frame

		struct
		{
			int offset = 0;
			size_t size = 0;

		} update_range;
		bool is_dirty = false;

		int append_buffer_data(char* data, size_t size)
		{
			size_t required_size = buffer_data.size() + size;
			int offset = (int)buffer_data.size();

			buffer_data.resize(required_size);
			memcpy(&buffer_data[offset], data, size);

			return offset;
		}

		std::vector<int> append_buffer_data(char* data, size_t size, size_t count)
		{
			int offset = (int)buffer_data.size();

			size_t required_size = buffer_data.size() + size * count;
			buffer_data.resize(required_size);
			
			std::vector<int> result;

			for (size_t i = 0; i < count; i++)
			{
				memcpy(&buffer_data[offset], data, size);
				result.push_back(offset);

				offset += size;
			}

			return result;
		}

		void create_buffer(char* data, size_t size)
		{
			if (buffer_id.id != 0)
				sg_destroy_buffer(buffer_id);

			sg_buffer_desc desc{};
			desc.data = { data, size };
			desc.size = size;
			desc.type = type;
			desc.usage = usage;

			buffer_id = sg_make_buffer(desc);
		}

		void merge_update_range(int offset, size_t size)
		{
			// Calculate the end points of the intervals
			int end1 = offset + size;
			int end2 = update_range.offset + update_range.size;

			// Find the new offset as the minimum of the current and updated offsets
			int new_offset = std::min(offset, update_range.offset);

			// Find the new size as the difference between the maximum end points and the new offset
			size_t new_size = std::max(end1, end2) - new_offset;

			// Update the update_range with the new merged interval
			update_range.offset = new_offset;
			update_range.size = new_size;
		}

	public:

		using range_id = size_t;

		buffer() = default;
		buffer(sg_usage usage, sg_buffer_type type) : usage(usage), type(type) {}

		operator bool() const
		{
			return buffer_id.id != 0;
		}

		void apply(range_id range_id, sg_bindings& bindings, size_t vertex_bindings_index = 0)
		{
			if (type == SG_BUFFERTYPE_INDEXBUFFER)
			{
				bindings.index_buffer = buffer_id;
				bindings.index_buffer_offset = ranges[range_id].offset;
			}
			else
			{
				bindings.vertex_buffers[vertex_bindings_index] = buffer_id;
				bindings.vertex_buffer_offsets[vertex_bindings_index] = ranges[range_id].offset;
			}
		}

		range_id append(char* data, size_t size)
		{
			buffer_range range{ -1, size };

			if (usage == SG_USAGE_IMMUTABLE)
			{
				range.offset = append_buffer_data(data, size);

				create_buffer(buffer_data.data(), buffer_data.size());
			}
			else
			{
				bool is_over_capacity = buffer_data.capacity() < buffer_data.size() + size;

				if (is_over_capacity)
				{
					if (buffer_data.capacity() == 0)
					{
						buffer_data.reserve(256);
						is_over_capacity = buffer_data.capacity() < buffer_data.size() + size;
					}

					while (is_over_capacity)
					{
						buffer_data.reserve(buffer_data.capacity() * 2);
						is_over_capacity = buffer_data.capacity() < buffer_data.size() + size;
					}

					range.offset = append_buffer_data(data, size);

					create_buffer(nullptr, buffer_data.capacity());

					sg_append_buffer(buffer_id, { buffer_data.data(), buffer_data.size() });
				}
				else
				{
					range.offset = append_buffer_data(data, size);
					int sg_offset = sg_append_buffer(buffer_id, { data, size });

					assert(sg_offset == range.offset);
				}
			}

			range_id result = ranges.size();
			ranges.push_back(std::move(range));

			is_appended = true;

			return result;
		}

		std::vector<range_id> append(char* data, size_t size, size_t count)
		{
			std::vector<range_id> result;

			auto append_buffer_data_and_create_ranges = [this](char* data, size_t size, size_t count)
			{
				std::vector<range_id> result;
				for (auto offset : append_buffer_data(data, size, count))
				{
					result.push_back(ranges.size());
					ranges.push_back({ offset, size });
				}
				return result;
			};

			if (usage == SG_USAGE_IMMUTABLE)
			{
				result = append_buffer_data_and_create_ranges(data, size, count);

				create_buffer(buffer_data.data(), buffer_data.size());
			}
			else
			{
				bool is_over_capacity = buffer_data.capacity() < buffer_data.size() + size * count;

				if (is_over_capacity)
				{
					if (buffer_data.capacity() == 0)
					{
						buffer_data.reserve(256);
						is_over_capacity = buffer_data.capacity() < buffer_data.size() + size * count;
					}

					while (is_over_capacity)
					{
						buffer_data.reserve(buffer_data.capacity() * 2);
						is_over_capacity = buffer_data.capacity() < buffer_data.size() + size * count;
					}

					result = append_buffer_data_and_create_ranges(data, size, count);

					create_buffer(nullptr, buffer_data.capacity());

					sg_append_buffer(buffer_id, { buffer_data.data(), buffer_data.size() });
				}
				else
				{
					result = append_buffer_data_and_create_ranges(data, size, count);
				}
			}

			is_appended = true;

			return result;
		}

		size_t get_data_size(range_id range_id)
		{
			return ranges[range_id].size;
		}

		// provide data pointer for in-place update
		void update_inplace(range_id range_id, char** data_ptr)
		{
			assert(usage != SG_USAGE_IMMUTABLE);

			auto [offset, size] = ranges[range_id];

			*data_ptr = &buffer_data[offset];

			is_dirty = true;

			merge_update_range(offset, size);
		}

		void update(range_id range_id, char* data)
		{
			assert(usage != SG_USAGE_IMMUTABLE);

			auto [offset, size] = ranges[range_id];

			memcpy(&buffer_data[offset], data, size);

			is_dirty = true;

			merge_update_range(offset, size);
		}

		void remove(range_id range_id)
		{
			// TODO
		}

		void flush()
		{
			if (is_dirty && !is_appended)
			{
				sg_update_buffer(buffer_id, { &buffer_data[update_range.offset], update_range.size });
				is_dirty = false;
			}
			is_appended = false;
		}
	};

	static uint32_t draw_buffer_id_counter = 1;

	struct instanced_element
	{
		float model[16];
		float color[4];
	};

	struct buffer_data
	{
		sg_pipeline pipeline = {};
		sg_bindings bindings = {};
		size_t draw_elements = 0;

		buffer* vertex_buffer = nullptr;
		buffer::range_id vertex_buffer_id = 0;

		buffer* index_buffer = nullptr;
		buffer::range_id index_buffer_id = 0;
	};

	struct buffer_data_instanced : public buffer_data
	{
		buffer instance_buffer;
		std::vector<buffer::range_id> instances;
	};

	enum class shader_type
	{
		basic,
		basic_instanced
	};

	struct pipeline_desc
	{
		sg_primitive_type type;
		bool index_buffer = false;
		shader_type shader;

		bool operator<(const pipeline_desc& o) const
		{
			if (o.type != type)
				return type < o.type;
			if (o.index_buffer != index_buffer)
				return index_buffer < o.index_buffer;
			return shader < o.shader;
		}
	};

	struct buffer_desc // TODO
	{
		sg_usage usage;
		sg_buffer_type type;

		bool operator<(const buffer_desc& o) const
		{
			if (o.type != type)
				return type < o.type;
			return usage < o.usage;
		}
	};

	struct
	{
		sg_shader basic_instanced;
		sg_shader basic;
		std::map<draw_buffer_id, buffer_data_instanced> buffer_data_instanced;
		std::map<draw_buffer_id, buffer_data> buffer_data;
		sg_pass_action pass_action;

		std::map<pipeline_desc, sg_pipeline> pipeline_cache;

		std::map<buffer_desc, buffer> buffer_cache;

	} state;

	sg_pipeline_desc get_pipeline_desc_basic_instanced(sg_primitive_type type, bool index_buffer)
	{
		sg_pipeline_desc pip_desc = {};
		pip_desc.primitive_type = type;
		pip_desc.shader = state.basic_instanced;
		if (index_buffer)
			pip_desc.index_type = SG_INDEXTYPE_UINT16;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_position].buffer_index = 0;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model0].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model0].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model1].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model1].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model2].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model2].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model3].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_model3].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_color].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_instanced_vs_color].buffer_index = 1;
		pip_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
		pip_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;

		return pip_desc;
	}

	sg_pipeline_desc get_pipeline_desc_basic(sg_primitive_type type, bool index_buffer)
	{
		sg_pipeline_desc pip_desc = {};
		pip_desc.primitive_type = type;
		pip_desc.shader = state.basic;
		if (index_buffer)
			pip_desc.index_type = SG_INDEXTYPE_UINT16;
		pip_desc.layout.attrs[ATTR_basic_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
		pip_desc.layout.attrs[ATTR_basic_vs_position].buffer_index = 0;

		return pip_desc;
	}

	sg_pipeline_desc get_pipeline_desc(pipeline_desc desc)
	{
		switch (desc.shader)
		{
		case shader_type::basic:
			return get_pipeline_desc_basic(desc.type, desc.index_buffer);
		case shader_type::basic_instanced:
			return get_pipeline_desc_basic_instanced(desc.type, desc.index_buffer);
		}
		return {};
	}

	sg_pipeline create_pipeline(pipeline_desc desc)
	{
		if (state.pipeline_cache.count(desc))
			return state.pipeline_cache[desc];

		state.pipeline_cache[desc] = sg_make_pipeline(get_pipeline_desc(desc));

		return state.pipeline_cache[desc];
	}

	std::pair<buffer*, buffer::range_id> create_buffer(buffer_desc desc, char* data, size_t size)
	{
		if (!state.buffer_cache.count(desc))
			state.buffer_cache[desc] = buffer(desc.usage, desc.type);

		buffer& buffer = state.buffer_cache[desc];

		return { &buffer, buffer.append(data, size) };
	}

	void setup_draw_sg()
	{
		state.basic_instanced = sg_make_shader(basic_instanced_shader_desc(sg_query_backend()));
		state.basic = sg_make_shader(basic_shader_desc(sg_query_backend()));

		//state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
		//state.pass_action.colors[0].clear_value = { 0.2f, 0.3f, 0.3f, 1.0f };
	}

	buffer_data_instanced create_buffer_data_instanced(const char* name,
													   int32_t draw_elements,
													   float* vertices,
													   size_t vertices_count,
													   uint16_t* indices,
													   size_t indices_count,
		                                               sg_primitive_type type,
													   sg_usage usage, // TODO same usage for both vertex and index buffer
													   size_t instances_count)
	{
		buffer_data_instanced result{};

		size_t vertex_buffer_size = 2 * sizeof(float) * vertices_count;
		std::tie(result.vertex_buffer, result.vertex_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_VERTEXBUFFER }, (char*)vertices, vertex_buffer_size);

		if (indices)
		{
			size_t index_buffer_size = 2 * sizeof(uint16_t) * indices_count;
			std::tie(result.index_buffer, result.index_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_INDEXBUFFER }, (char*)indices, index_buffer_size);
		}

		result.instance_buffer = buffer(SG_USAGE_DYNAMIC, SG_BUFFERTYPE_VERTEXBUFFER);

		result.pipeline = create_pipeline({ type, indices != nullptr, shader_type::basic_instanced });
		result.draw_elements = draw_elements;

		instanced_element default_instance = {};
		auto ranges = result.instance_buffer.append((char*)&default_instance, sizeof(instanced_element), instances_count);
		for (auto r : ranges)
			result.instances.push_back(r);

		return result;
	}

	buffer_data create_buffer_data(const char* name,
				                   int32_t draw_elements,
						           float* vertices,
						           size_t vertices_count,
							       uint16_t* indices,
							       size_t indices_count,
							       sg_primitive_type type,
		                           sg_usage usage)
	{
		buffer_data result{};

		size_t vertex_buffer_size = 2 * sizeof(float) * vertices_count;
		std::tie(result.vertex_buffer, result.vertex_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_VERTEXBUFFER }, (char*)vertices, vertex_buffer_size);

		if (indices)
		{
			size_t index_buffer_size = 2 * sizeof(uint16_t) * indices_count;
			std::tie(result.index_buffer, result.index_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_INDEXBUFFER }, (char*)indices, index_buffer_size);
		}

		result.pipeline = create_pipeline({ type, indices != nullptr, shader_type::basic });
		result.draw_elements = draw_elements;

		return result;
	}

	mesh_data create_mesh_rectangle()
	{
		mesh_data result;
		result.vertices.assign({
			0.5f,  0.5f, // top right
			0.5f, -0.5f, // bottom right
			-0.5f, -0.5f, // bottom left
			-0.5f,  0.5f, // top left
		});

		result.indices.assign({ 0, 1, 3, 2 });

		return result;
	}

	mesh_data create_mesh_circle(size_t count)
	{
		mesh_data result;

		result.vertices.resize(count * 2);
		for (size_t i = 0; i < count; i++)
		{
			float angle = 2.0f * frame::PI * (float)i / (float)(count);

			result.vertices[i * 2] = std::cos(angle) * 0.5f;
			result.vertices[i * 2 + 1] = std::sin(angle) * 0.5f;
		}

		std::vector<uint16_t> indices;
		for (uint16_t i = 0; i < count - 2; i++)
		{
			result.indices.push_back(0);
			result.indices.push_back(i + 1);
			result.indices.push_back(i + 2);
		}

		return result;
	}

	mesh_data create_mesh_circle_no_index(size_t count)
	{
		mesh_data result;

		float step = 2.0f * frame::PI / count;

		std::vector<vec2> temp;
		for (int i = 0; i < count; ++i)
		{
			float angle = i * step;
			temp.push_back({ 0.5f * cos(angle), 0.5f * sin(angle) });
		}

		int triangleCount = count - 2;
		for (int i = 0; i < triangleCount; i++)
		{
			result.vertices.push_back(temp[0].x);
			result.vertices.push_back(temp[0].y);
			result.vertices.push_back(temp[i + 1].x);
			result.vertices.push_back(temp[i + 1].y);
			result.vertices.push_back(temp[i + 2].x);
			result.vertices.push_back(temp[i + 2].y);
		}

		return result;
	}

	draw_buffer_id create_draw_buffer_instanced(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage)
	{
		draw_buffer_id id{ draw_buffer_id_counter++ };

		size_t elements_count = mesh.indices ? mesh.indices_count : mesh.vertices_count;

		state.buffer_data_instanced[id] = std::move(create_buffer_data_instanced(name,
			                                                                     elements_count,
																				 mesh.vertices, 
																				 mesh.vertices_count,
																			     mesh.indices,
																				 mesh.indices_count,
																				 type,
																				 usage,
	                                                                             0));

		return id;
	}

	draw_buffer_id create_draw_buffer_instanced(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage, size_t instances_count)
	{
		draw_buffer_id id{ draw_buffer_id_counter++ };

		size_t elements_count = mesh.indices ? mesh.indices_count : mesh.vertices_count;

		state.buffer_data_instanced[id] = std::move(create_buffer_data_instanced(name,
																			  	 elements_count,
																			 	 mesh.vertices,
																			     mesh.vertices_count,
																				 mesh.indices,
																				 mesh.indices_count,
																				 type,
																				 usage,
																				 instances_count));

		return id;
	}

	draw_buffer_id create_instanced_rectangle()
	{
		return create_draw_buffer_instanced("rectangle", create_mesh_rectangle(), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_DYNAMIC);
	}

	draw_buffer_id create_instanced_circle(size_t count)
	{
		return create_draw_buffer_instanced("circle", create_mesh_circle(count), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_DYNAMIC);
	}

	hmm_mat4 create_hmm_transform(frame::vec2 position, float rotation, frame::vec2 size)
	{
		auto scale = HMM_Scale({ size.x, size.y, 0.0f });
		auto rotate = HMM_Rotate(rotation, HMM_Vec3(0.0f, 0.0f, 1.0f));
		auto translate = HMM_Translate({ position.x, position.y, 0.0f });
		return HMM_MultiplyMat4(HMM_MultiplyMat4(translate, rotate), scale);
	}

	hmm_mat4 create_hmm_transform(const frame::mat3& transform)
	{
		hmm_mat4 result = HMM_Mat4d(1.0f);

		result.Elements[0][0] = transform.data[0];
		result.Elements[0][1] = transform.data[3];
		result.Elements[0][3] = transform.data[6];

		result.Elements[1][0] = transform.data[1];
		result.Elements[1][1] = transform.data[4];
		result.Elements[1][3] = transform.data[7];

		result.Elements[3][0] = transform.data[2];
		result.Elements[3][1] = transform.data[5];
		result.Elements[3][3] = transform.data[8];

		return result;
	}

	hmm_mat4 create_projection_view_matrix()
	{
		hmm_mat4 view = create_hmm_transform(frame::get_world_transform());
		hmm_mat4 projection = HMM_Orthographic(0.0f, sapp_widthf(), sapp_heightf(), 0.0f, 0.0f, 100.0f);

		return HMM_MultiplyMat4(projection, view);
	}

	size_t add_draw_instance(draw_buffer_id id, const hmm_mat4& model, frame::col4 color)
	{
		auto& data = state.buffer_data_instanced[id];

		instanced_element instance{};

		memcpy(instance.model, model.Elements, sizeof(model.Elements));
		memcpy(instance.color, &color, sizeof(color));

		data.instances.push_back(data.instance_buffer.append((char*)&instance, sizeof(instanced_element)));

		return data.instances.size() - 1;
	}


	size_t add_draw_instance(draw_buffer_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		return add_draw_instance(id, create_hmm_transform(position, rotation, size), color);
	}

	size_t add_draw_instance(draw_buffer_id id, const frame::mat3& transform, frame::col4 color)
	{
		return add_draw_instance(id, create_hmm_transform(transform), color);
	}

	void remove_draw_instance(draw_buffer_id id, size_t index)
	{
		// TODO
	}

	void update_draw_instance(draw_buffer_id id, size_t index, const hmm_mat4& model, frame::col4 color)
	{
		auto& data = state.buffer_data_instanced[id];

		instanced_element* instance;
		data.instance_buffer.update_inplace(data.instances[index], (char**)&instance);

		memcpy(instance->model, model.Elements, sizeof(model.Elements));
		memcpy(instance->color, &color, sizeof(color));
	}

	// fast position update
	void update_draw_instance(draw_buffer_id id, size_t index, const frame::vec2& position, const frame::col4& color)
	{
		auto& data = state.buffer_data_instanced[id];

		instanced_element* instance;
		data.instance_buffer.update_inplace(data.instances[index], (char**)&instance);

		//  0  1  2  3
		//	4  5  6  7
		//	8  9 10 11
		// 12 13 14 15

		instance->model[12] = position.x;// / instance->model[0];
		instance->model[13] = position.y;// / instance->model[5];

		memcpy(instance->color, &color, sizeof(color));
	}

	void update_draw_instance(draw_buffer_id id, size_t index, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		update_draw_instance(id, index, create_hmm_transform(position, rotation, size), color);
	}

	void update_draw_instance(draw_buffer_id id, size_t index, const frame::mat3& transform, frame::col4 color)
	{
		update_draw_instance(id, index, create_hmm_transform(transform), color);
	}

	void draw_buffer_data_instanced(buffer_data_instanced& data, size_t count = 0)
	{
		if (data.instances.size() == 0)
			return;

		data.vertex_buffer->flush();
		data.vertex_buffer->apply(data.vertex_buffer_id, data.bindings, 0);

		data.instance_buffer.flush();
		data.instance_buffer.apply(data.instances[0], data.bindings, 1);

		if (data.index_buffer)
		{
			data.index_buffer->flush();
			data.index_buffer->apply(data.index_buffer_id, data.bindings);
		}

		sg_apply_pipeline(data.pipeline);
		sg_apply_bindings(&data.bindings);

		auto projection_view = create_projection_view_matrix();

		basic_instanced_vs_params_t vs_params;
		memcpy(vs_params.view_projection, projection_view.Elements, sizeof(projection_view.Elements));
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_basic_instanced_vs_params, SG_RANGE(vs_params));

		sg_draw(0, data.draw_elements, count == 0 ? data.instances.size() : count);

		//state.rect.instances = 0;
	}

	void draw_buffer_instanced(draw_buffer_id id)
	{
		draw_buffer_data_instanced(state.buffer_data_instanced[id]);
	}

	void draw_buffer_instanced(draw_buffer_id id, size_t count)
	{
		draw_buffer_data_instanced(state.buffer_data_instanced[id], count);
	}

	draw_buffer_id create_draw_buffer(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage)
	{
		draw_buffer_id id{ draw_buffer_id_counter++ };

		size_t elements_count = mesh.indices ? mesh.indices_count : mesh.vertices_count;

		state.buffer_data[id] = std::move(create_buffer_data(name,
								        					 elements_count,
															 mesh.vertices,
															 mesh.vertices_count,
															 mesh.indices,
															 mesh.indices_count,
															 type,
			                                                 usage));

		return id;
	}

	void draw_buffer(draw_buffer_id id, const hmm_mat4& model , frame::col4 color)
	{
		auto& data = state.buffer_data[id];

		data.vertex_buffer->flush();
		data.vertex_buffer->apply(data.vertex_buffer_id, data.bindings, 0);

		if (data.index_buffer)
		{
			data.index_buffer->flush();
			data.index_buffer->apply(data.index_buffer_id, data.bindings);
		}

		sg_apply_pipeline(data.pipeline);
		sg_apply_bindings(&data.bindings);

		hmm_mat4 projection_view = create_projection_view_matrix();

		basic_vs_params_t vs_params;
		memcpy(vs_params.mvp, HMM_MultiplyMat4(projection_view, model).Elements, sizeof(model.Elements));
		memcpy(vs_params.color, color.data.rgba, sizeof(color.data.rgba));
		
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_basic_vs_params, SG_RANGE(vs_params));

		sg_draw(0, data.draw_elements, 1);
	}

	void draw_buffer(draw_buffer_id id, frame::col4 color)
	{
		draw_buffer(id, HMM_Mat4d(1.0f), color);
	}

	void draw_buffer(draw_buffer_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		draw_buffer(id, create_hmm_transform(position, rotation, size), color);
	}

	void draw_buffer(draw_buffer_id id, const frame::mat3& transform, frame::col4 color)
	{
		draw_buffer(id, create_hmm_transform(transform), color);
	}
}
