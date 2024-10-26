#pragma once
#include "framework.h"
#include "sokol_gl.h"

namespace frame
{
	using pipeline_t = uint32_t;

	struct desc_hasher
	{
		std::size_t operator()(const frame::image_target_desc& key) const
		{
			return (size_t(key.pixel_format) << 16) + (size_t(key.depth_stencil) << 8) + key.samples;
		}
	};

	struct pipeline_manager_sg
	{
		pipeline_t make_pipeline(const sg_pipeline_desc& desc);
		void destroy_pipeline(pipeline_t pipeline);

		void apply_pipeline(pipeline_t pipeline);
		void apply_pass(const image_target_desc& desc);

	private:
		sg_pipeline make_pipeline(const image_target_desc& pass_data, const sg_pipeline_desc& pip_template);

		pipeline_t pip_counter = 1;

		struct pipeline_data
		{
			sg_pipeline_desc pip_template;
			sg_pipeline* pip_pass = nullptr;

			std::unordered_map<image_target_desc, sg_pipeline, desc_hasher> pipelines;
		};

		std::unordered_map<pipeline_t, pipeline_data> pipelines_data;
	};

	struct manager_sg
	{
		void initialize();

		image_t create_render_target(uint32_t width, uint32_t height, const image_target_desc& desc);

		void begin_pass(image_t target);
		void begin_pass_clear(image_t target, const col4& color);
		void begin_default_pass();
		void end_pass();

		void set_default_pass_clear_color(const col4& color);
		col4 get_default_pass_clear_color();

		bool is_default_pass();
		std::pair<uint32_t, uint32_t> get_pass_size();

		bool is_target(image_t target);
		const image_target_desc& get_target_desc(image_t target);
		const image_target_desc& get_default_target_desc();

	private:
		struct pass_data_t
		{
			sg_pass pass = {};
			sgl_context context = {};
			image_target_desc desc = {};
			uint32_t width = 0;
			uint32_t height = 0;
		};

		std::map<image_t, pass_data_t> passes;

		pass_data_t* current_pass = nullptr;
		bool is_in_pass = false;

		sg_pass_action default_pass_action = {};
		image_target_desc default_target_desc = {};
	};
}