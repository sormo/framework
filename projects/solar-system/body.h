#pragma once
#include <string>
#include <kepler_orbit.h>
#include "trajectory_resolutions.h"

enum class body_type
{
    none,
    star,
    planet,
    dward_planet,
    lagrange_point,
    barycenter,
    moon,
    minor_planet,
    comet
};

std::string get_body_type_to_string(body_type type_value);
body_type get_body_type_from_string(const std::string& type_string);

struct body_node
{
    std::string name;
    body_type type = body_type::none;
    std::string group;
    kepler_orbit orbit;
    double radius = 0.0; // mean radius of body in km
    double period = 0.0;
    double mass = 0.0;
    double density = 0.0;
    //double dimensions[3] = {};
    std::string dimensions_str;
    double inclination = 0.0;

    bool is_major_body = false; // whether it's most massive body in a case parent is barycenter

    body_node* parent = nullptr;
    std::vector<body_node*> childs;

    trajectory_resolutions trajectory;
    frame::rectangle name_text_rectangle; // cached name text rectangle in screen coordinates
    frame::vec2 current_position; // cache used during drawing

    body_node* get_main_body(); // main body is body orbiting solar-system baarycenter
    frame::vec3d get_absolute_position();
    frame::vec3d get_main_body_position(); // this will return position relative to main body
};