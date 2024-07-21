#pragma once
#include <kepler_orbit.h>
#include <framework.h>
#include "trajectory_resolutions.h"
#include "body.h"
#include "quadtree.h"
#include "body_info.h"
#include "camera.h"
#include <string>
#include <vector>
#include <queue>

struct bodies_tree
{
    std::vector<body_node> bodies;
    body_node* parent = nullptr;

    std::vector<body_node*> query(const frame::vec2& query_point, float query_radius);
    void load(std::vector<const char*> json_datas);
    void step(double time_delta);
    void clear();

    void update_current_positions(const quadtree::query_result_type& parents);
private:

    bool is_barycenter(const body_node& body);
};
