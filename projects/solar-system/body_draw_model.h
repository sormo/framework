#include "body.h"
#include <framework.h>
#include <utils.h>
#include <zip.h>
#include <set>

struct body_draw_model
{
	void setup(const std::vector<char>& models_zip_data);
	bool draw(body_node& body, float radius, const frame::col4& color);

private:
	void setup_pip();
	bool setup_bind(body_node& body);

	bool load_buffer(body_node& body);
	bool load_model(body_node& body);

	// TODO introduce numerical body id in body.h
	using body_model_id = std::string;

	std::set<body_model_id> available_models;

	std::unordered_map<body_model_id, frame::model_t> models_cache;
	std::unordered_map<body_model_id, sg_buffer> buffer_cache;

	std::optional<frame::zip_archive> models_zip;

	sg_pipeline pip_model;
	sg_bindings bind_model;
	sg_bindings bind_model_test;
	size_t element_count = 0;

	std::optional<body_model_id> bound_body;
};
