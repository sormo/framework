#include "body.h"
#include "commons.h"
#include <framework.h>
#include <buffer_sg.h>
#include <utils.h>
#include <zip.h>
#include <set>
#include <optional>

struct body_draw_model
{
	body_draw_model();

	void setup();
	bool draw(body_node& body, float radius, const frame::col4& color);

	void fetch_models(commons::bodies_included_type type);

private:
	void setup_pip();
	bool setup_bind(body_node& body);

	bool load_buffer(body_node& body);
	std::optional<frame::model_t> load_model(body_node& body);

	// TODO introduce numerical body id in body.h
	using body_model_id = std::string;

	struct model_data
	{
		buffer_sg::range_id range_id = 0;
		size_t element_count = 0;
	};

	std::unordered_map<body_model_id, model_data> buffer_cache;

	struct model_zip
	{
		std::set<body_model_id> available_models;
		frame::zip_archive zip;
	};

	std::optional<model_zip> models_100;
	std::optional<model_zip> models_50;
	std::optional<model_zip> models_10;
	std::optional<model_zip> models_moons;

	buffer_sg buffer_model;

	sg_pipeline pip_model;
	sg_bindings bind_model;
	size_t element_count = 0;

	std::optional<body_model_id> bound_body;
};
