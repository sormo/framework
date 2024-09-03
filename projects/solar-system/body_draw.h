#pragma once
#include "body.h"
#include "body_info.h"
#include "bodies_tree.h"
#include "body_draw_shaded.h"
#include "body_draw_model.h"
#include <framework.h>

struct body_draw
{
	void setup(body_color& cols);
	void setup_bodies(bodies_tree& bodies);
	void setup_models(commons::bodies_included_type type);

	void draw(body_node* body, bool is_root);

private:
	// if view is centered on body, we will draw names with depth, otherwise names are always in front
	void draw_names(const quadtree::query_result_type& parents, bool is_view);
	void draw_trajectories(const quadtree::query_result_type& parents, body_color& colors, body_node* stationary_body);
	void draw_points(const quadtree::query_result_type& parents, body_color& colors);

	void draw_body(body_node& body, size_t& point_counter, body_color& colors);
	void draw_sun(body_node& body, size_t& point_counter, body_color& colors);

	void draw_world_bodies(body_node* root);
	void draw_main_body(body_node* body);

	void initialize_instance_buffer(bodies_tree& bodies);

	body_draw_shaded body_drawer_shaded;
	body_draw_model body_drawer_model;
	body_color* colors;

	frame::draw_buffer_id points_instance_buffer = 0;

	frame::draw_buffer_id planet_circle = 0;
};
