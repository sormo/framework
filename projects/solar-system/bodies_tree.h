#pragma once
#include <kepler_orbit.h>
#include <framework.h>
#include "trajectory_resolutions.h"
#include <string>
#include <vector>
#include <set>

struct body_node
{
    std::string name;
    std::string group;
    kepler_orbit orbit;
    double radius = 0.0; // mean radius of body in km

    body_node* parent = nullptr;
    std::vector<body_node*> childs;

    trajectory_resolutions trajectory;

    frame::vec3d get_absolute_position();
};

struct bodies_tree
{
    std::vector<body_node> bodies;
    body_node* parent;

    std::vector<body_node*> query(const frame::vec2& query_point, float query_radius);
    void load(const std::vector<std::string>& files);
    void draw_names(std::set<body_node*>& parents);
    void draw_trajectories(std::set<body_node*>& parents);
    void step(double time_delta);

private:
    bool is_barycenter(const body_node& body);
};
