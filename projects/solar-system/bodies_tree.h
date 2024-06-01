#pragma once
#include <kepler_orbit.h>
#include <framework.h>
#include "trajectory_resolutions.h"
#include "body.h"
#include "quadtree.h"
#include "body_info.h"
#include <string>
#include <vector>

struct bodies_tree
{
    std::vector<body_node> bodies;
    body_node* parent;

    std::vector<body_node*> query(const frame::vec2& query_point, float query_radius);
    void load(std::vector<const char*> json_datas);
    void draw(quadtree::query_result_type& parents, body_color& colors);
    void step(double time_delta);
    void clear();

private:
    void draw_names(quadtree::query_result_type& parents);
    void draw_trajectories(quadtree::query_result_type& parents, body_color& colors);
    void draw_points(quadtree::query_result_type& parents, body_color& colors);

    void update_current_positions(quadtree::query_result_type& parents);

    bool is_barycenter(const body_node& body);

    frame::draw_buffer_id points_instance_buffer;
};
