#pragma once

struct body_data
{
    std::string name;
    kepler_orbit orbit;

    body_data* parent = nullptr;
    std::vector<body_data*> childs;

    trajectory_resolutions trajectory;
};

struct bodies_tree
{
    std::vector<body_data> bodies;
    body_data* parent;
};
