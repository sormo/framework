#include "drawing_sg.h"
#include "utils.h"
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <map>
#include <string>
#include <vector>
#include "basic_instanced.glsl.h"
#include "basic.glsl.h"
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

namespace frame
{
	static uint32_t draw_buffer_id_counter = 1;

	struct instanced_element
	{
		float model[16];
		float color[4];
	};

	struct buffer_data_instanced
	{
		sg_pipeline pipeline = {};
		sg_bindings bindings = {};
		std::vector<instanced_element> array;
		size_t instances_max = 100;
		size_t instances = 0;
		size_t draw_elements = 0;
		bool is_dirty = false; // signalizes whether buffer should be updated before drawing
	};

	struct buffer_data
	{
		sg_pipeline pipeline = {};
		sg_bindings bindings = {};
		size_t draw_elements = 0;
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

		std::map<buffer_desc, sg_buffer> buffer_cache; // TODO use sg_append_buffer and set offset to sg_bindings stored in buffer_data*

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

	std::pair<sg_buffer, int> create_buffer(buffer_desc desc, const void* data, size_t size)
	{
		sg_buffer buffer{};

		if (!state.buffer_cache.count(desc))
		{
			sg_buffer_desc sg_desc = {};
			sg_desc.type = desc.type;
			sg_desc.usage = desc.usage;
			sg_desc.size = 20'000'000; // TODO
			sg_desc.data = { nullptr, 20'000'000 };

			auto buffer = sg_make_buffer(sg_desc);

			state.buffer_cache[desc] = sg_make_buffer(sg_desc);

			sg_update_buffer(buffer, { data, size });

			return { buffer, 0 };
		}
		else
		{
			auto buffer = state.buffer_cache[desc];

			return { buffer, sg_append_buffer(buffer, { data, size }) };
		}
	}

	void setup_draw_sg()
	{
		state.basic_instanced = sg_make_shader(basic_instanced_shader_desc(sg_query_backend()));
		state.basic = sg_make_shader(basic_shader_desc(sg_query_backend()));

		state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
		state.pass_action.colors[0].clear_value = { 0.2f, 0.3f, 0.3f, 1.0f };
	}

	buffer_data_instanced create_buffer_data_instanced(const char* name,
		                                               int32_t draw_elements,
		                                               float* vertices,
		                                               size_t vertices_count,
		                                               uint16_t* indices,
		                                               size_t indices_count,
										               sg_primitive_type type,
		                                               sg_usage usage, // TODO same usage for both vertex and instance buffer
		                                               size_t instances_max)
	{
		buffer_data_instanced result{};

		result.instances_max = instances_max;
		result.array.resize(result.instances_max);

		size_t vertex_buffer_size = 2 * sizeof(float) * vertices_count;
		auto [vertex_buffer, vertex_buffer_offset] = create_buffer({ usage, SG_BUFFERTYPE_VERTEXBUFFER }, vertices, vertex_buffer_size);

		result.bindings.vertex_buffers[0] = vertex_buffer;
		result.bindings.vertex_buffer_offsets[0] = vertex_buffer_offset;

		size_t instance_buffer_size = result.array.size() * sizeof(instanced_element);
		//auto [instance_buffer, instance_buffer_offset] = create_buffer({ usage, SG_BUFFERTYPE_VERTEXBUFFER }, nullptr, instance_buffer_size);

		//result.bindings.vertex_buffers[1] = instance_buffer;
		//result.bindings.vertex_buffer_offsets[1] = instance_buffer_offset;

		sg_buffer_desc instance_buffer_desc = {};
		instance_buffer_desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
		instance_buffer_desc.usage = usage;
		instance_buffer_desc.size = instance_buffer_size;
		instance_buffer_desc.data = { nullptr, instance_buffer_size };

		auto instance_buffer = sg_make_buffer(instance_buffer_desc);

		result.bindings.vertex_buffers[1] = instance_buffer;
		result.bindings.vertex_buffer_offsets[1] = 0;

		if (indices)
		{
			size_t index_buffer_size = 2 * sizeof(uint16_t) * indices_count;
			auto [index_buffer, index_buffer_offset] = create_buffer({ usage, SG_BUFFERTYPE_INDEXBUFFER }, indices, index_buffer_size);

			result.bindings.index_buffer = index_buffer;
			result.bindings.index_buffer_offset = index_buffer_offset;
		}

		result.pipeline = create_pipeline({ type, indices != nullptr, shader_type::basic_instanced });
		result.draw_elements = draw_elements;

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

		std::string name_str(name);
		std::string name_vertex = name_str + "-vertices";
		std::string name_indices = name_str + "-indices";

		size_t vertex_buffer_size = 2 * sizeof(float) * vertices_count;
		auto [vertex_buffer, vertex_buffer_offset] = create_buffer({ usage, SG_BUFFERTYPE_VERTEXBUFFER }, vertices, vertex_buffer_size);

		result.bindings.vertex_buffers[0] = vertex_buffer;
		result.bindings.vertex_buffer_offsets[0] = vertex_buffer_offset;

		if (indices)
		{
			size_t index_buffer_size = 2 * sizeof(uint16_t) * indices_count;
			auto [index_buffer, index_buffer_offset] = create_buffer({ usage, SG_BUFFERTYPE_INDEXBUFFER }, indices, index_buffer_size);

			result.bindings.index_buffer = index_buffer;
			result.bindings.index_buffer_offset = index_buffer_offset;
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

	draw_buffer_id create_draw_buffer_instanced(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage, size_t max_count)
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
																				 max_count));

		return id;
	}

	draw_buffer_id create_instanced_rectangle()
	{
		return create_draw_buffer_instanced("rectangle", create_mesh_rectangle(), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_DYNAMIC, 1000);
	}

	draw_buffer_id create_instanced_circle(size_t count)
	{
		return create_draw_buffer_instanced("circle", create_mesh_circle(count), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_DYNAMIC, 1000);
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

		auto index = data.instances;
		memcpy(data.array[index].model, model.Elements, sizeof(model.Elements));
		memcpy(data.array[index].color, &color, sizeof(color));

		data.instances++;
		data.is_dirty = true;

		return index;
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
		auto& data = state.buffer_data_instanced[id];

		data.array.erase(std::begin(data.array) + index);
		data.is_dirty = true;
	}

	void update_draw_instance(draw_buffer_id id, size_t index, const hmm_mat4& model, frame::col4 color)
	{
		auto& data = state.buffer_data_instanced[id];

		memcpy(data.array[index].model, model.Elements, sizeof(model.Elements));
		memcpy(data.array[index].color, &color, sizeof(color));

		data.is_dirty = true;
	}

	void update_draw_instance(draw_buffer_id id, size_t index, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		update_draw_instance(id, index, create_hmm_transform(position, rotation, size), color);
	}

	void update_draw_instance(draw_buffer_id id, size_t index, const frame::mat3& transform, frame::col4 color)
	{
		update_draw_instance(id, index, create_hmm_transform(transform), color);
	}

	void draw_buffer_data_instanced(buffer_data_instanced& data)
	{
		if (data.instances == 0)
			return;

		sg_apply_pipeline(data.pipeline);
		sg_apply_bindings(&data.bindings);

		if (data.is_dirty)
		{
			sg_range update_range;
			update_range.ptr = data.array.data();
			update_range.size = sizeof(instanced_element) * data.instances;

			sg_update_buffer(data.bindings.vertex_buffers[1], &update_range);

			data.is_dirty = false;
		}

		auto projection_view = create_projection_view_matrix();

		basic_instanced_vs_params_t vs_params;
		memcpy(vs_params.view_projection, projection_view.Elements, sizeof(projection_view.Elements));
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_basic_instanced_vs_params, &SG_RANGE(vs_params));

		sg_draw(0, data.draw_elements, data.instances);

		//state.rect.instances = 0;
	}

	void draw_buffer_instanced(draw_buffer_id id)
	{
		draw_buffer_data_instanced(state.buffer_data_instanced[id]);
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

		sg_apply_pipeline(data.pipeline);
		sg_apply_bindings(&data.bindings);

		hmm_mat4 projection_view = create_projection_view_matrix();

		basic_vs_params_t vs_params;
		memcpy(vs_params.mvp, HMM_MultiplyMat4(projection_view, model).Elements, sizeof(model.Elements));
		memcpy(vs_params.color, color.data.rgba, sizeof(color.data.rgba));
		
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_basic_vs_params, &SG_RANGE(vs_params));

		sg_draw(0, data.draw_elements, 1);
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
