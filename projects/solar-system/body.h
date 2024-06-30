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
    comet,
    spacecraft
};

std::string get_body_type_to_string(body_type type_value);
body_type get_body_type_from_string(const std::string& type_string);

enum class lagrangian
{
    l1,
    l2,
    l3,
    l4,
    l5,
};

std::string get_lagrangian_to_string(lagrangian lagr);

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

    struct
    {
        bool is_major_body = false; // whether it's most massive body in a case parent is barycenter
        body_node* major_body = nullptr; // major body in a case this is barycenter

    } system;

    body_node* parent = nullptr;
    std::vector<body_node*> childs;

    trajectory_resolutions trajectory;
    frame::rectangle name_text_rectangle; // cached name text rectangle in screen coordinates
    frame::vec2 current_position; // cache used during drawing

    body_node* get_main_body(); // main body is body orbiting solar-system baarycenter
    frame::vec3d get_absolute_position();
    frame::vec3d get_main_body_position(); // this will return position relative to main body
    frame::vec3d get_lagrangian(lagrangian lagr);
};

// 2 body system providing minor body
// minor body is less massive body
// major body is parent body (not necessarily direct parent) which is more massive
struct minor_system
{
    minor_system(body_node& minor);

    const frame::vec3d& get_major_position();
    const frame::vec3d& get_major_velocity();
    double get_major_mass();
    kepler_orbit* get_major_orbit(); // may return null if this is not barycentric system

    const frame::vec3d& get_minor_position();
    const frame::vec3d& get_minor_velocity();
    double get_minor_mass();
    kepler_orbit& get_minor_orbit();

    body_node* minor;
    body_node* major;
    body_node* barycenter = nullptr;
};
