#include "drawing_sg.h"
#include "buffer_sg.h"
#include "utils.h"
#include <xxh3.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#define SOKOL_SHAPE_IMPL
#include <sokol_shape.h>
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include "shaders/basic_depth_instanced.glsl.h"
#include "shaders/basic_depth.glsl.h"
#include "shaders/basic_instanced.glsl.h"
#include "shaders/basic.glsl.h"
#include "shaders/sshapes.glsl.h"
#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

namespace frame
{
	///////////////////////////////////////////
	/// BUFFERS ///////////////////////////////
	///////////////////////////////////////////

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

		buffer_sg* vertex_buffer = nullptr;
		buffer_sg::range_id vertex_buffer_id = 0;

		buffer_sg* index_buffer = nullptr;
		buffer_sg::range_id index_buffer_id = 0;
	};

	struct buffer_data_instanced : public buffer_data
	{
		buffer_sg instance_buffer;
		std::vector<buffer_sg::range_id> instances;
	};

	enum class shader_type
	{
		basic,
		basic_instanced,
		basic_depth,
		basic_depth_instanced,
	};

	struct pipeline_desc
	{
		sg_primitive_type type = SG_PRIMITIVETYPE_TRIANGLES;
		bool index_buffer = false;
		shader_type shader = shader_type::basic;
		uint8_t stride_in_bytes = 0;

		bool operator<(const pipeline_desc& o) const
		{
			if (o.type != type)
				return type < o.type;
			if (o.index_buffer != index_buffer)
				return index_buffer < o.index_buffer;
			if (o.shader != shader)
				return shader < o.shader;
			return stride_in_bytes < o.stride_in_bytes;
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
		sg_shader basic_depth_instanced;
		sg_shader basic_depth;
		sg_shader basic_instanced;
		sg_shader basic;
		std::unordered_map<draw_buffer_id, buffer_data_instanced> buffer_data_instanced;
		std::unordered_map<draw_buffer_id, buffer_data> buffer_data;
		sg_pass_action pass_action;

		std::map<pipeline_desc, sg_pipeline> pipeline_cache;

		std::map<buffer_desc, buffer_sg> buffer_cache;

		// --- sshapes ---
		sg_pipeline sshape_pip;
		sg_buffer sshape_vbuf;
		sg_buffer sshape_ibuf;

		sshape_element_range_t sshape_draw_box;
		sshape_element_range_t sshape_draw_sphere;
		sshape_element_range_t sshape_draw_cylinder;

	} state;

	sg_pipeline_desc get_pipeline_desc_common(sg_primitive_type type, bool index_buffer)
	{
		sg_pipeline_desc pip_desc = {};
		pip_desc.primitive_type = type;
		
		if (index_buffer)
			pip_desc.index_type = SG_INDEXTYPE_UINT16;

		pip_desc.depth.write_enabled = true;
		pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;

		pip_desc.colors[0].blend.enabled = true;
		pip_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		pip_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		pip_desc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
		pip_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		pip_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
		pip_desc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;

		return pip_desc;
	}

	sg_pipeline_desc get_pipeline_desc_basic_depth_instanced(sg_primitive_type type, bool index_buffer)
	{
		sg_pipeline_desc pip_desc = get_pipeline_desc_common(type, index_buffer);
		pip_desc.shader = state.basic_depth_instanced;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_position].buffer_index = 0;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model0].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model0].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model1].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model1].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model2].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model2].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model3].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_model3].buffer_index = 1;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_color].format = SG_VERTEXFORMAT_FLOAT4;
		pip_desc.layout.attrs[ATTR_basic_depth_instanced_vs_color].buffer_index = 1;
		pip_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
		pip_desc.layout.buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE;

		return pip_desc;
	}

	sg_pipeline_desc get_pipeline_desc_basic_instanced(sg_primitive_type type, bool index_buffer)
	{
		sg_pipeline_desc pip_desc = get_pipeline_desc_common(type, index_buffer);
		pip_desc.shader = state.basic_instanced;
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

	sg_pipeline_desc get_pipeline_desc_basic_depth(sg_primitive_type type, bool index_buffer, int stride_in_bytes)
	{
		sg_pipeline_desc pip_desc = get_pipeline_desc_common(type, index_buffer);
		pip_desc.shader = state.basic_depth;
		// position attribute in shader (starts at offset 0, it is taken from buffer at index 0 and is two floats)
		pip_desc.layout.attrs[ATTR_basic_depth_vs_position].format = SG_VERTEXFORMAT_FLOAT3;
		pip_desc.layout.attrs[ATTR_basic_depth_vs_position].buffer_index = 0;
		pip_desc.layout.attrs[ATTR_basic_depth_vs_position].offset = 0;
		// single buffer with positions at index 0
		pip_desc.layout.buffers[0].stride = stride_in_bytes;
		pip_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
		pip_desc.layout.buffers[0].step_rate = 0;

		return pip_desc;
	}

	sg_pipeline_desc get_pipeline_desc_basic(sg_primitive_type type, bool index_buffer, int stride_in_bytes)
	{
		sg_pipeline_desc pip_desc = get_pipeline_desc_common(type, index_buffer);
		pip_desc.shader = state.basic;
		// position attribute in shader (starts at offset 0, it is taken from buffer at index 0 and is two floats)
		pip_desc.layout.attrs[ATTR_basic_vs_position].format = SG_VERTEXFORMAT_FLOAT2;
		pip_desc.layout.attrs[ATTR_basic_vs_position].buffer_index = 0;
		pip_desc.layout.attrs[ATTR_basic_vs_position].offset = 0;
		// single buffer with positions at index 0
		pip_desc.layout.buffers[0].stride = stride_in_bytes;
		pip_desc.layout.buffers[0].step_func = SG_VERTEXSTEP_PER_VERTEX;
		pip_desc.layout.buffers[0].step_rate = 0;

		return pip_desc;
	}

	sg_pipeline_desc get_pipeline_desc(pipeline_desc desc)
	{
		switch (desc.shader)
		{
		case shader_type::basic:
			return get_pipeline_desc_basic(desc.type, desc.index_buffer, desc.stride_in_bytes);
		case shader_type::basic_instanced:
			return get_pipeline_desc_basic_instanced(desc.type, desc.index_buffer);
		case shader_type::basic_depth:
			return get_pipeline_desc_basic_depth(desc.type, desc.index_buffer, desc.stride_in_bytes);
		case shader_type::basic_depth_instanced:
			return get_pipeline_desc_basic_depth_instanced(desc.type, desc.index_buffer);
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

	std::pair<buffer_sg*, buffer_sg::range_id> create_buffer(buffer_desc desc, char* data, size_t size)
	{
		if (!state.buffer_cache.count(desc))
			state.buffer_cache[desc] = buffer_sg(desc.usage, desc.type);

		buffer_sg& buffer = state.buffer_cache[desc];

		return { &buffer, buffer.append(data, size) };
	}

	void setup_sshapes()
	{
		// pipeline
		sg_pipeline_desc pipeline_desc = {};
		pipeline_desc.shader = sg_make_shader(sshapes_shader_desc(sg_query_backend()));
		pipeline_desc.layout.buffers[0] = sshape_vertex_buffer_layout_state();
		pipeline_desc.layout.attrs[ATTR_sshapes_vs_position] = sshape_position_vertex_attr_state();
		pipeline_desc.layout.attrs[ATTR_sshapes_vs_normal] = sshape_normal_vertex_attr_state();
		// currently not used
		//pipeline_desc.layout.attrs[ATTR_sshapes_vs_texcoord] = sshape_texcoord_vertex_attr_state();
		//pipeline_desc.layout.attrs[ATTR_sshapes_vs_color0] = sshape_color_vertex_attr_state();
		pipeline_desc.index_type = SG_INDEXTYPE_UINT16;
		pipeline_desc.cull_mode = SG_CULLMODE_NONE;
		pipeline_desc.depth.write_enabled = true;
		pipeline_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
		pipeline_desc.label = "sshapes-pipeline";

		pipeline_desc.colors[0].blend.enabled = true;
		pipeline_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
		pipeline_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline_desc.colors[0].blend.op_rgb = SG_BLENDOP_ADD;
		pipeline_desc.colors[0].blend.src_factor_alpha = SG_BLENDFACTOR_ONE;
		pipeline_desc.colors[0].blend.dst_factor_alpha = SG_BLENDFACTOR_ZERO;
		pipeline_desc.colors[0].blend.op_alpha = SG_BLENDOP_ADD;

		state.sshape_pip = sg_make_pipeline(&pipeline_desc);

		// sshapes

		static sshape_vertex_t vertices[6 * 1024];
		static uint16_t indices[16 * 1024];

		sshape_buffer_t buffer = {};
		buffer.vertices.buffer = SSHAPE_RANGE(vertices);
		buffer.indices.buffer = SSHAPE_RANGE(indices);

		sshape_box_t box_desc = {};
		buffer = sshape_build_box(&buffer, &box_desc);
		state.sshape_draw_box = sshape_element_range(&buffer);

		sshape_sphere_t sphere_desc = {};
		sphere_desc.slices = 36;
		sphere_desc.stacks = 20;
		buffer = sshape_build_sphere(&buffer, &sphere_desc);
		state.sshape_draw_sphere = sshape_element_range(&buffer);

		sshape_cylinder_t cylinder_desc = {};
		cylinder_desc.slices = 36;
		cylinder_desc.stacks = 20;
		buffer = sshape_build_cylinder(&buffer, &cylinder_desc);
		state.sshape_draw_cylinder = sshape_element_range(&buffer);

		auto vbuf_desc = sshape_vertex_buffer_desc(&buffer);
		state.sshape_vbuf = sg_make_buffer(&vbuf_desc);

		auto ibuf_desc = sshape_index_buffer_desc(&buffer);
		state.sshape_ibuf = sg_make_buffer(&ibuf_desc);
	}

	void setup_draw_sg()
	{
		state.basic_depth_instanced = sg_make_shader(basic_depth_instanced_shader_desc(sg_query_backend()));
		state.basic_depth = sg_make_shader(basic_depth_shader_desc(sg_query_backend()));
		state.basic_instanced = sg_make_shader(basic_instanced_shader_desc(sg_query_backend()));
		state.basic = sg_make_shader(basic_shader_desc(sg_query_backend()));

		setup_sshapes();

		//state.pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
		//state.pass_action.colors[0].clear_value = { 0.2f, 0.3f, 0.3f, 1.0f };
	}

	buffer_data_instanced create_buffer_data_instanced(const char* name,
													   size_t draw_elements,
													   float* vertices,
													   size_t vertices_count,
													   uint16_t* indices,
													   size_t indices_count,
													   mesh_t mesh_type,
		                                               sg_primitive_type primitive_type,
													   sg_usage usage, // TODO same usage for both vertex and index buffer
													   size_t instances_count)
	{
		buffer_data_instanced result{};

		size_t floats_per_vertex = mesh_type == mesh_t::basic ? 2 : 3;

		size_t vertex_buffer_size = floats_per_vertex * sizeof(float) * vertices_count;
		std::tie(result.vertex_buffer, result.vertex_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_VERTEXBUFFER }, (char*)vertices, vertex_buffer_size);

		if (indices)
		{
			// TODO really 2 * ??
			size_t index_buffer_size = 2 * sizeof(uint16_t) * indices_count;
			std::tie(result.index_buffer, result.index_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_INDEXBUFFER }, (char*)indices, index_buffer_size);
		}

		result.instance_buffer = buffer_sg(SG_USAGE_DYNAMIC, SG_BUFFERTYPE_VERTEXBUFFER);

		auto shader = mesh_type == mesh_t::basic ? shader_type::basic_instanced : shader_type::basic_depth_instanced;

		result.pipeline = create_pipeline({ primitive_type, indices != nullptr, shader });
		result.draw_elements = draw_elements;

		instanced_element default_instance = {};
		auto ranges = result.instance_buffer.append((char*)&default_instance, sizeof(instanced_element), instances_count);
		for (auto r : ranges)
			result.instances.push_back(r);

		return result;
	}

	buffer_data create_buffer_data(const char* name,
				                   size_t draw_elements,
						           float* vertices,
						           size_t vertices_count,
							       uint16_t* indices,
							       size_t indices_count,
								   mesh_t mesh_type,
							       sg_primitive_type primitive_type,
		                           sg_usage usage,
								   uint8_t stride_in_bytes)
	{
		buffer_data result{};

		size_t floats_per_vertex = mesh_type == mesh_t::basic ? 2 : 3;

		size_t vertex_buffer_size = floats_per_vertex * sizeof(float) * vertices_count;
		std::tie(result.vertex_buffer, result.vertex_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_VERTEXBUFFER }, (char*)vertices, vertex_buffer_size);

		if (indices)
		{
			// TODO really 2 * sizeof(uint16_t) ??
			size_t index_buffer_size = 2 * sizeof(uint16_t) * indices_count;
			std::tie(result.index_buffer, result.index_buffer_id) = create_buffer({ usage, SG_BUFFERTYPE_INDEXBUFFER }, (char*)indices, index_buffer_size);
		}

		auto shader = mesh_type == mesh_t::basic ? shader_type::basic : shader_type::basic_depth;

		result.pipeline = create_pipeline({ primitive_type, indices != nullptr, shader, stride_in_bytes });
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
			float angle = 2.0f * (float)frame::PI * (float)i / (float)(count);

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

	mesh_data create_mesh_circle(size_t count, float depth)
	{
		mesh_data result;

		result.vertices.resize(count * 3);
		for (size_t i = 0; i < count; i++)
		{
			float angle = 2.0f * (float)frame::PI * (float)i / (float)(count);

			result.vertices[i * 3 + 0] = std::cos(angle) * 0.5f;
			result.vertices[i * 3 + 1] = std::sin(angle) * 0.5f;
			result.vertices[i * 3 + 2] = depth;
		}

		std::vector<uint16_t> indices;
		for (uint16_t i = 0; i < count - 2; i++)
		{
			result.indices.push_back(0);
			result.indices.push_back(i + 1);
			result.indices.push_back(i + 2);
		}

		result.type = mesh_t::depth;

		return result;
	}

	mesh_data create_mesh_circle_no_index(size_t count)
	{
		mesh_data result;

		float step = 2.0f * (float)frame::PI / count;

		std::vector<vec2> temp;
		for (int i = 0; i < (int)count; ++i)
		{
			float angle = i * step;
			temp.push_back({ 0.5f * cos(angle), 0.5f * sin(angle) });
		}

		int triangleCount = (int)count - 2;
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
																				 mesh.type,
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
																				 mesh.type,
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

	HMM_Mat4 create_hmm_transform(frame::vec2 position, float rotation, frame::vec2 size)
	{
		auto scale = HMM_Scale({ size.x, size.y, 1.0f });
		auto rotate = HMM_Rotate_RH(rotation, HMM_Vec3{ 0.0f, 0.0f, 1.0f });
		auto translate = HMM_Translate({ position.x, position.y, 0.0f });
		return HMM_MulM4(HMM_MulM4(translate, rotate), scale);
	}

	HMM_Mat4 create_hmm_transform(frame::vec3 position, float rotation, frame::vec2 size)
	{
		auto scale = HMM_Scale({ size.x, size.y, 1.0f });
		auto rotate = HMM_Rotate_RH(rotation, HMM_Vec3{ 0.0f, 0.0f, 1.0f });
		auto translate = HMM_Translate({ position.x, position.y, position.z });
		return HMM_MulM4(HMM_MulM4(translate, rotate), scale);
	}

	HMM_Mat4 create_hmm_transform(frame::vec3 position, float rotation, frame::vec3 size)
	{
		auto scale = HMM_Scale({ size.x, size.y, size.z });
		auto rotate = HMM_Rotate_RH(rotation, HMM_Vec3{ 0.0f, 0.0f, 1.0f });
		auto translate = HMM_Translate({ position.x, position.y, position.z });
		return HMM_MulM4(HMM_MulM4(translate, rotate), scale);
	}

	HMM_Mat4 create_hmm_transform(const frame::mat3& transform)
	{
		HMM_Mat4 result = HMM_M4D(1.0f);

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

	HMM_Mat4 create_hmm_transform(const frame::mat4& transform)
	{
		return transform.data;
	}

	HMM_Mat4 create_projection_view_matrix()
	{
		HMM_Mat4 view = frame::get_world_transform().data;
		HMM_Mat4 projection = HMM_Orthographic_RH_NO(0.0f, sapp_widthf(), sapp_heightf(), 0.0f, -max_depth, max_depth);

		return HMM_MulM4(projection, view);
	}

	HMM_Mat4 create_world_mvp(frame::vec2 position, float rotation, frame::vec2 size)
	{
		return HMM_MulM4(create_projection_view_matrix(), create_hmm_transform(position, rotation, size));
	}

	HMM_Mat4 create_world_mvp(frame::vec3 position, float rotation, frame::vec2 size)
	{
		return HMM_MulM4(create_projection_view_matrix(), create_hmm_transform(position, rotation, size));
	}

	HMM_Mat4 create_world_mvp(frame::vec3 position, float rotation, frame::vec3 size)
	{
		return HMM_MulM4(create_projection_view_matrix(), create_hmm_transform(position, rotation, size));
	}

	HMM_Mat4 create_world_mvp(const frame::mat4& transform)
	{
		return HMM_MulM4(create_projection_view_matrix(), transform.data);
	}

	HMM_Vec3 to_hmm(const frame::vec3& v)
	{
		return { v.x, v.y, v.z };
	}

	size_t add_draw_instance(draw_buffer_id id, const HMM_Mat4& model, frame::col4 color)
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

	size_t add_draw_instance(draw_buffer_id id, frame::vec3 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		return add_draw_instance(id, create_hmm_transform(position, rotation, size), color);
	}

	size_t add_draw_instance(draw_buffer_id id, const frame::mat4& transform, frame::col4 color)
	{
		return add_draw_instance(id, transform.data, color);
	}

	void remove_draw_instance(draw_buffer_id id, size_t index)
	{
		// TODO
	}

	void update_draw_instance(draw_buffer_id id, size_t index, const HMM_Mat4& model, frame::col4 color)
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

	void update_draw_instance(draw_buffer_id id, size_t index, const frame::vec3& position, const frame::col4& color)
	{
		auto& data = state.buffer_data_instanced[id];

		instanced_element* instance;
		data.instance_buffer.update_inplace(data.instances[index], (char**)&instance);

		instance->model[12] = position.x;
		instance->model[13] = position.y;
		instance->model[14] = position.z;

		memcpy(instance->color, &color, sizeof(color));
	}

	void update_draw_instance(draw_buffer_id id, size_t index, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		update_draw_instance(id, index, create_hmm_transform(position, rotation, size), color);
	}

	void update_draw_instance(draw_buffer_id id, size_t index, const frame::mat4& transform, frame::col4 color)
	{
		update_draw_instance(id, index, transform.data, color);
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

		sg_draw(0, (int)data.draw_elements, count == 0 ? (int)data.instances.size() : (int)count);

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

	draw_buffer_id create_draw_buffer(const char* name, mesh mesh, sg_primitive_type type, sg_usage usage, uint8_t stride_in_bytes)
	{
		draw_buffer_id id{ draw_buffer_id_counter++ };

		size_t elements_count = mesh.indices ? mesh.indices_count : mesh.vertices_count;

		// TODO how does stride work with indices ???
		// doing ceil to draw as much as possible, test whether this is ok, possibly add elements_count as function argument
		if (stride_in_bytes)
		{
			float stride_divisor = (float)stride_in_bytes / (mesh.type == mesh_t::basic ? (float)sizeof(frame::vec2) : (float)sizeof(frame::vec3));
			elements_count = (size_t)std::ceil((float)elements_count / stride_divisor);
		}

		state.buffer_data[id] = create_buffer_data(name,
			elements_count,
			mesh.vertices,
			mesh.vertices_count,
			mesh.indices,
			mesh.indices_count,
			mesh.type,
			type,
			usage,
			stride_in_bytes);

		return id;
	}

	void draw_buffer(draw_buffer_id id, const HMM_Mat4& mvp, frame::col4 color)
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

		// TODO this can be wrong if using different shader
		basic_vs_params_t vs_params;
		memcpy(vs_params.mvp, mvp.Elements, sizeof(mvp.Elements));
		memcpy(vs_params.color, color.data.rgba, sizeof(color.data.rgba));

		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_basic_vs_params, SG_RANGE(vs_params));

		sg_draw(0, (int)data.draw_elements, 1);
	}

	void draw_buffer(draw_buffer_id id, frame::col4 color)
	{
		draw_buffer(id, create_projection_view_matrix(), color);
	}

	void draw_buffer(draw_buffer_id id, frame::vec2 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		draw_buffer(id, HMM_MulM4(create_projection_view_matrix(), create_hmm_transform(position, rotation, size)), color);
	}

	void draw_buffer(draw_buffer_id id, frame::vec3 position, float rotation, frame::vec2 size, frame::col4 color)
	{
		draw_buffer(id, HMM_MulM4(create_projection_view_matrix(), create_hmm_transform(position, rotation, size)), color);
	}

	void draw_buffer(draw_buffer_id id, const frame::mat4& transform, frame::col4 color)
	{
		draw_buffer(id, HMM_MulM4(create_projection_view_matrix(), transform.data), color);
	}

	void draw_buffers(const std::vector<draw_buffer_id>& ids, const std::vector<frame::mat4>& transforms, const std::vector<frame::col4>& colors)
	{
		HMM_Mat4 projection_view = create_projection_view_matrix();

		for (size_t i = 0; i < ids.size(); i++)
		{
			draw_buffer(ids[i], HMM_MulM4(projection_view, transforms[i].data), colors[i]);
		}
	}

	void draw_buffers(const std::vector<draw_buffer_id>& ids, const std::vector<HMM_Mat4>& transforms, const std::vector<frame::col4>& colors)
	{
		HMM_Mat4 projection_view = create_projection_view_matrix();

		for (size_t i = 0; i < ids.size(); i++)
		{
			draw_buffer(ids[i], HMM_MulM4(projection_view, transforms[i]), colors[i]);
		}
	}

	void remove_buffer(draw_buffer_id id)
	{
		// TODO
	}

	HMM_Mat4 create_hmm_direction(const frame::vec3& direction, const frame::vec3& up_direction)
	{
		// special case - no orientation transformation needed
		if (direction == up_direction)
			return HMM_M4D(1.0f);

		auto yaxis = direction;

		auto zaxis = direction.cross(up_direction);
		zaxis.normalize();

		auto xaxis = direction.cross(zaxis);
		xaxis.normalize();

		HMM_Mat4 model = HMM_M4D(1.0f);

		model.Columns[0].X = xaxis.x;
		model.Columns[0].Y = xaxis.y;
		model.Columns[0].Z = xaxis.z;

		model.Columns[1].X = yaxis.x;
		model.Columns[1].Y = yaxis.y;
		model.Columns[1].Z = yaxis.z;

		model.Columns[2].X = zaxis.x;
		model.Columns[2].Y = zaxis.y;
		model.Columns[2].Z = zaxis.z;

		return model;
	}

	void apply_sshape_pipeline()
	{
		sg_apply_pipeline(state.sshape_pip);

		sg_bindings bindings = {};
		bindings.vertex_buffers[0] = state.sshape_vbuf;
		bindings.index_buffer = state.sshape_ibuf;
		sg_apply_bindings(&bindings);
	}

	void apply_sshape_uniform(const HMM_Mat4& transform, const frame::col4& color, sshapes_shading shading, const frame::vec3& light_position, const HMM_Mat4& model)
	{
		sshapes_vs_params_t vs_params = {};
		vs_params.color = color;
		vs_params.mvp = transform;
		vs_params.shading_type = (int)shading;
		vs_params.light_direction = light_position.normalized();
		vs_params.model = model;
		sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_sshapes_vs_params, SG_RANGE(vs_params));
	}

	void draw_cube(const HMM_Mat4& transform, const frame::col4& color, sshapes_shading shading, const frame::vec3& light_position, const HMM_Mat4& model)
	{
		apply_sshape_pipeline();
		apply_sshape_uniform(transform, color, shading, light_position, model);
		sg_draw(state.sshape_draw_box.base_element, state.sshape_draw_box.num_elements, 1);
	}

	void draw_sphere(const HMM_Mat4& transform, const frame::col4& color, sshapes_shading shading, const frame::vec3& light_position, const HMM_Mat4& model)
	{
		apply_sshape_pipeline();
		apply_sshape_uniform(transform, color, shading, light_position, model);
		sg_draw(state.sshape_draw_sphere.base_element, state.sshape_draw_sphere.num_elements, 1);
	}

	void draw_cylinder(const HMM_Mat4& transform, const frame::col4& color, sshapes_shading shading, const frame::vec3& light_position, const HMM_Mat4& model)
	{
		apply_sshape_pipeline();
		apply_sshape_uniform(transform, color, shading, light_position, model);
		sg_draw(state.sshape_draw_cylinder.base_element, state.sshape_draw_cylinder.num_elements, 1);
	}

	void draw_gizmo(const HMM_Mat4& transform, float axis_length, float axis_width)
	{
		apply_sshape_pipeline();

		auto scale = HMM_Scale({ axis_width, axis_length, axis_width });

		// y-axis - red
		auto yaxis = HMM_MulM4(transform, HMM_MulM4(HMM_Translate({ 0.0f, axis_length / 2.0f, 0.0f }), scale));
		apply_sshape_uniform(yaxis, col4::RED, sshapes_shading::none, {}, HMM_M4D(1.0f));
		sg_draw(state.sshape_draw_cylinder.base_element, state.sshape_draw_cylinder.num_elements, 1);

		// x-axis - green
		auto xaxis = HMM_MulM4(transform, HMM_MulM4(HMM_Translate({ axis_length / 2.0f, 0.0f, 0.0f }), HMM_MulM4(HMM_Rotate_RH(-PI / 2.0f, { 0.0f, 0.0f, 1.0f }), scale)));
		apply_sshape_uniform(xaxis, col4::GREEN, sshapes_shading::none, {}, HMM_M4D(1.0f));
		sg_draw(state.sshape_draw_cylinder.base_element, state.sshape_draw_cylinder.num_elements, 1);

		// z-axis - blue
		auto zaxis = HMM_MulM4(transform, HMM_MulM4(HMM_Translate({ 0.0f, 0.0f, axis_length/2.0f }), HMM_MulM4(HMM_Rotate_RH(PI / 2.0f, { 1.0f, 0.0f, 0.0f }), scale)));
		apply_sshape_uniform(zaxis, col4::BLUE, sshapes_shading::none, {}, HMM_M4D(1.0f));
		sg_draw(state.sshape_draw_cylinder.base_element, state.sshape_draw_cylinder.num_elements, 1);
	}
}
