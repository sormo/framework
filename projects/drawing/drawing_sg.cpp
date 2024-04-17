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

	struct
	{
		sg_shader basic_instanced;
		sg_shader basic;
		std::map<draw_buffer_id, buffer_data_instanced> buffer_data_instanced;
		std::map<draw_buffer_id, buffer_data> buffer_data;
		sg_pass_action pass_action;

	} state;

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
		                                               sg_usage usage,
		                                               size_t instances_max)
	{
		buffer_data_instanced result{};

		result.instances_max = instances_max;
		result.array.resize(result.instances_max);

		std::string name_str(name);
		std::string name_vertex = name_str + "-vertices";
		std::string name_instanced = name_str + "-instanced-array";
		std::string name_indices = name_str + "-indices";
		std::string name_pipeline = name_str + "-pipeline";

		sg_buffer_desc vdesc = {};
		vdesc.size = 2 * sizeof(float) * vertices_count;
		vdesc.data = { vertices, 2 * sizeof(float) * vertices_count };
		vdesc.label = name_vertex.c_str();

		result.bindings.vertex_buffers[0] = sg_make_buffer(vdesc);

		sg_buffer_desc adesc = {};
		adesc.usage = usage;
		adesc.size = result.array.size() * sizeof(instanced_element);
		//adesc.data = SG_RANGE(state.rect.array);
		adesc.label = name_instanced.c_str();

		result.bindings.vertex_buffers[1] = sg_make_buffer(adesc);

		if (indices)
		{
			sg_buffer_desc idesc = {};
			idesc.type = SG_BUFFERTYPE_INDEXBUFFER;
			idesc.size = 2 * sizeof(float) * indices_count;
			idesc.data = { indices, 2 * sizeof(float) * indices_count };
			idesc.label = name_indices.c_str();

			result.bindings.index_buffer = sg_make_buffer(idesc);
		}

		sg_pipeline_desc pip_desc = {};
		pip_desc.primitive_type = type;
		pip_desc.shader = state.basic_instanced;
		if (indices)
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
		pip_desc.label = name_pipeline.c_str();

		result.pipeline = sg_make_pipeline(pip_desc);

		result.draw_elements = draw_elements;

		return result;
	}

	buffer_data create_buffer_data(const char* name,
				                   int32_t draw_elements,
						           float* vertices,
						           size_t vertices_count,
							       uint16_t* indices,
							       size_t indices_count,
							       sg_primitive_type type)
	{
		buffer_data result{};

		std::string name_str(name);
		std::string name_vertex = name_str + "-vertices";
		std::string name_indices = name_str + "-indices";
		std::string name_pipeline = name_str + "-pipeline";

		sg_buffer_desc vdesc = {};
		vdesc.size = 2 * sizeof(float) * vertices_count;
		vdesc.data = { vertices, 2 * sizeof(float) * vertices_count };
		vdesc.label = name_vertex.c_str();

		result.bindings.vertex_buffers[0] = sg_make_buffer(vdesc);

		if (indices)
		{
			sg_buffer_desc idesc = {};
			idesc.type = SG_BUFFERTYPE_INDEXBUFFER;
			idesc.size = 2 * sizeof(float) * indices_count;
			idesc.data = { indices, 2 * sizeof(float) * indices_count };
			idesc.label = name_indices.c_str();

			result.bindings.index_buffer = sg_make_buffer(idesc);
		}

		sg_pipeline_desc pip_desc = {};
		pip_desc.primitive_type = type;
		pip_desc.shader = state.basic;
		if (indices)
			pip_desc.index_type = SG_INDEXTYPE_UINT16;
		pip_desc.layout.attrs[ATTR_basic_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
		pip_desc.layout.attrs[ATTR_basic_vs_position].buffer_index = 0;
		pip_desc.label = name_pipeline.c_str();

		result.pipeline = sg_make_pipeline(pip_desc);

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
		//vertices.assign({ 
		//	0.5f,  0.5f, 0.0f,  // top right
		//	0.5f, -0.5f, 0.0f,  // bottom right
		//	-0.5f, -0.5f, 0.0f,  // bottom left
		//	-0.5f,  0.5f, 0.0f,  // top left
		//});

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

	draw_buffer_id create_draw_buffer(const char* name, mesh mesh, sg_primitive_type type)
	{
		draw_buffer_id id{ draw_buffer_id_counter++ };

		size_t elements_count = mesh.indices ? mesh.indices_count : mesh.vertices_count;

		state.buffer_data[id] = std::move(create_buffer_data(name,
								        					 elements_count,
															 mesh.vertices,
															 mesh.vertices_count,
															 mesh.indices,
															 mesh.indices_count,
															 type));

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
