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
    void load(const std::vector<std::string>& files);
    void draw_names(std::set<body_node*>& parents);
    void draw_trajectories(std::set<body_node*>& parents, body_color& colors);
    void step(double time_delta);

private:
    bool is_barycenter(const body_node& body);
};
