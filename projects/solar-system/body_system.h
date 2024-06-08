#pragma once
#include <framework.h>
#include "bodies_tree.h"
#include "quadtree.h"
#include "body_info.h"

struct body_system
{
	void setup();
	void draw();
	void update();

	void set_follow_body(body_node* body);

	body_node* query(const frame::vec2& world_position, float radius_in_pixels);
	void set_info(body_node* body);
	void setup_bodies(commons::bodies_included_type type);

private:
	void setup_colors(std::map<std::string, std::vector<char>>& files);
	void setup_info(std::map<std::string, std::vector<char>>& files);
	void create_quadtree(float max_size, float min_size);
	void setup_quadtree(const char* cache_filename, const std::vector<char>& cache_file_data);
	void load_bodies_tree(std::vector<std::vector<char>*> files);
	void step_bodies_tree();
	void draw_bodies_tree();
	void draw_distance_legend();
	void draw_current_time();

	bodies_tree bodies;
	body_view body_draw;

	quadtree tree;
	body_color body_color_data;
	body_info info;
};
