#pragma once
#include <kepler_orbit.h>
#include <framework.h>
#include "trajectory_resolutions.h"
#include "body.h"
#include "body_info.h"
#include <string>
#include <vector>
#include <set>

struct bodies_tree
{
    std::vector<body_node> bodies;
    body_node* parent;

    std::vector<body_node*> query(const frame::vec2& query_point, float query_radius);
    void load(std::vector<const char*> json_datas);
    void draw(std::set<body_node*>& parents, body_color& colors);
    void step(double time_delta);
    void clear();

private:
    void draw_names(std::set<body_node*>& parents);
    void draw_trajectories(std::set<body_node*>& parents, body_color& colors);
    void draw_points(std::set<body_node*>& parents, body_color& colors);

    void update_current_positions(std::set<body_node*>& parents);

    bool is_barycenter(const body_node& body);

    frame::draw_buffer_id points_instance_buffer;
};
