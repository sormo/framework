#pragma once
#include <framework.h>
#include "bodies_tree.h"
#include "quadtree.h"
#include "body_info.h"
#include "body_draw.h"

struct body_system
{
	void setup();
	void draw(body_node* view_body = nullptr);
	void update();

	body_node* query(const frame::vec2& world_position, float radius_in_pixels);
	void set_info(body_node* body);
	void setup_bodies(commons::bodies_included_type type, std::function<void()> onSetup = nullptr);

	body_node* get_body(const char* name);

	body_info info;

private:
	void setup_colors(std::map<std::string, std::vector<char>>& files);
	void setup_info(std::map<std::string, std::vector<char>>& files);
	void create_quadtree(float max_size, float min_size);
	void setup_quadtree(const char* cache_filename, const std::vector<char>& cache_file_data);
	void load_bodies_tree(std::vector<std::vector<char>*> files);
	void step_bodies_tree();
	
	void update_current_positions(body_node* main_body);

	void draw_lagrangians(body_node* body);
	void draw_distance_legend();
	void draw_current_time();

	bodies_tree bodies;
	body_draw body_drawer;

	quadtree tree;
	body_color body_color_data;
	
};
