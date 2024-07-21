#pragma once
#include "body.h"
#include "body_info.h"
#include "bodies_tree.h"
#include <framework.h>

struct body_draw_shaded
{
	void setup();
	void draw(body_node& body, float radius, const frame::col4& color);

private:
	sg_pipeline pip_planet;
	sg_bindings bind_planet;
};

struct body_draw
{
	void setup(body_color& cols);
	void setup_bodies(bodies_tree& bodies);

	void draw(body_node* body, bool is_root);

private:

	void draw_names(const quadtree::query_result_type& parents);
	void draw_trajectories(const quadtree::query_result_type& parents, body_color& colors, body_node* stationary_body);
	void draw_points(const quadtree::query_result_type& parents, body_color& colors);

	void draw_world_bodies(body_node* root);
	void draw_main_body(body_node* body);

	void initialize_instance_buffer(bodies_tree& bodies);

	body_draw_shaded body_drawer;
	body_color* colors;

	frame::draw_buffer_id points_instance_buffer = 0;
};
