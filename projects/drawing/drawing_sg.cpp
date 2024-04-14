#include "drawing_sg.h"
#include "utils.h"
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <map>
#include <string>
#include <vector>
#include "basic_instanced.glsl.h"
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

namespace frame
{
	static uint32_t instance_id_counter = 1;

	struct instanced_element
	{
		float model[16];
		float color[4];
	};

	struct instanced_data
	{
		sg_pipeline pipeline = {};
		sg_bindings bindings = {};
		std::vector<instanced_element> array;
		size_t instances_max = 100;
		size_t instances = 0;
		size_t draw_elements = 0;
		bool is_dirty = false; // signalizes whether buffer should be updated before drawing
	};

	struct
	{
		sg_shader basic_instanced;
		std::map<instanced_id, instanced_data> instanced;
		sg_pass_action pass_action;

	} state;

	void setup_instanced()
	{
		state.basic_instanced = sg_make_shader(basic_instanced_shader_desc(sg_query_backend()));

		state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
		state.pass_action.colors[0].clear_value = { 0.2f, 0.3f, 0.3f, 1.0f };
	}

	instanced_data create_instanced_data(char* name,
		                                 int32_t draw_elements,
		                                 float* vertices,
		                                 size_t vertices_count,
		                                 uint16_t* indices,
		                                 size_t indices_count,
										 sg_primitive_type type,
		                                 sg_usage usage,
		                                 size_t instances_max)
	{
		instanced_data result{};

		result.instances_max = instances_max;
		result.array.resize(result.instances_max);

		std::string name_str(name);
		std::string name_vertex = name_str + "-vertices";
		std::string name_instanced = name_str + "-instanced-array";
		std::string name_indices = name_str + "-indices";
		std::string name_pipeline = name_str + "-pipeline";

		sg_buffer_desc vdesc = {};
		vdesc.size = sizeof(float) * vertices_count;
		vdesc.data = { vertices, sizeof(float) * vertices_count };
		vdesc.label = name_vertex.c_str();

		result.bindings.vertex_buffers[0] = sg_make_buffer(vdesc);

		sg_buffer_desc adesc = {};
		adesc.usage = usage;
		adesc.size = result.array.size() * sizeof(instanced_element);
		//adesc.data = SG_RANGE(state.rect.array);
		adesc.label = name_instanced.c_str();

		result.bindings.vertex_buffers[1] = sg_make_buffer(adesc);

		sg_buffer_desc idesc = {};
		idesc.type = SG_BUFFERTYPE_INDEXBUFFER;
		idesc.size = sizeof(float) * indices_count;
		idesc.data = { indices, sizeof(float) * indices_count };
		idesc.label = name_indices.c_str();

		result.bindings.index_buffer = sg_make_buffer(idesc);

		sg_pipeline_desc pip_desc = {};
		pip_desc.primitive_type = type;
		pip_desc.shader = state.basic_instanced;
		pip_desc.index_type = SG_INDEXTYPE_UINT16;
		pip_desc.layout.attrs[ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
		pip_desc.layout.attrs[ATTR_vs_position].buffer_index = 0;
		pip_desc.layout.attrs[ATTR_vs_model0].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_vs_model0].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_vs_model1].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_vs_model1].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_vs_model2].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_vs_model2].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_vs_model3].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_vs_model3].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_vs_color].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_vs_color].buffer_index = 1;
		pip_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
		pip_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;
		pip_desc.label = name_pipeline.c_str();

		result.pipeline = sg_make_pipeline(pip_desc);

		result.draw_elements = draw_elements;

		return result;
	}

	instanced_id create_instanced_rectangle()
	{
		float vertices[] =
		{
			0.5f,  0.5f, 0.0f,  // top right
			0.5f, -0.5f, 0.0f,  // bottom right
		   -0.5f, -0.5f, 0.0f,  // bottom left
		   -0.5f,  0.5f, 0.0f,  // top left
		};

		uint16_t indices[] =
		{
			0, 1, 3, 2
		};

		instanced_id id{ instance_id_counter++ };

		state.instanced[id] = std::move(create_instanced_data("rectangle", 4, vertices, 12, indices, 4, SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_DYNAMIC, 1000));

		return id;
	}

	instanced_id create_instanced_circle(size_t count)
	{
		std::vector<float> vertices(count * 3);
		for (size_t i = 0; i < count; i++)
		{
			float angle = 2.0f * frame::PI * (float)i / (float)(count);

			vertices[i * 3] = std::cos(angle);
			vertices[i * 3 + 1] = std::sin(angle);
			vertices[i * 3 + 2] = 0.0f;
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
			indices.push_back(0);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}
		//indices.assign({
		//	0, 1, 3, 2
		//});

		//std::reverse(std::begin(indices), std::end(indices));

		instanced_id id{ instance_id_counter++ };

		state.instanced[id] = std::move(create_instanced_data("circle", (count - 2)*3, vertices.data(), vertices.size(), indices.data(), indices.size(), SG_PRIMITIVETYPE_TRIANGLES, SG_USAGE_DYNAMIC, 10'000));

		return id;
	}

	hmm_mat4 create_model_matrix(frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		auto scale = HMM_Scale({ size.x, size.y, 0.0f });
		auto rotate = HMM_Rotate(rotation, HMM_Vec3(0.0f, 0.0f, 1.0f));
		auto translate = HMM_Translate({ position.x, position.y, 0.0f });
		return HMM_MultiplyMat4(HMM_MultiplyMat4(translate, rotate), scale);
	}

	size_t add_instanced(instanced_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		auto& data = state.instanced[id];

		hmm_mat4 model = create_model_matrix(position, rotation, size, color);

		auto index = data.instances;
		memcpy(data.array[index].model, model.Elements, sizeof(model.Elements));
		memcpy(data.array[index].color, &color, sizeof(color));

		data.instances++;
		data.is_dirty = true;

		return index;
	}

	void remove_instanced(instanced_id id, size_t index)
	{
		auto& data = state.instanced[id];

		data.array.erase(std::begin(data.array) + index);
		data.is_dirty = true;
	}

	void update_instanced(instanced_id id, size_t index, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		auto& data = state.instanced[id];

		hmm_mat4 model = create_model_matrix(position, rotation, size, color);

		memcpy(data.array[index].model, model.Elements, sizeof(model.Elements));
		memcpy(data.array[index].color, &color, sizeof(color));

		data.is_dirty = true;
	}

	void draw_instanced_data(instanced_data& data)
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

		hmm_mat4 view = HMM_Translate(HMM_Vec3(sapp_width() / 2.0f, sapp_height() / 2.0f, 0.0f));
		hmm_mat4 projection = HMM_Orthographic(0.0f, sapp_width(), 0.0f, sapp_height(), 0.0f, 100.0f);

		vs_params_t vs_params;
		memcpy(vs_params.view_projection, HMM_MultiplyMat4(projection, view).Elements, sizeof(projection.Elements));
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));

		sg_draw(0, data.draw_elements, data.instances);

		//state.rect.instances = 0;
	}

	void draw_instanced(instanced_id id)
	{
		draw_instanced_data(state.instanced[id]);
	}
}
