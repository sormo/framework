#include "body.h"
#include "unit.h"

using namespace frame;

static constexpr std::pair<std::string_view, body_type> body_type_mapping[] =
{
    { "", body_type::none },
    { "Star", body_type::star },
    { "Planet", body_type::planet },
    { "Dwarf Planet", body_type::dward_planet },
    { "Lagrange Point", body_type::lagrange_point },
    { "Barycenter", body_type::barycenter },
    { "Moon", body_type::moon },
    { "Minor Planet", body_type::minor_planet },
    { "Comet", body_type::comet },
    { "Spacecraft", body_type::spacecraft },
};

body_type get_body_type_from_string(const std::string& type_str)
{
    for (const auto& [name, type] : body_type_mapping)
    {
        if (type_str == name)
            return type;
    }
    return body_type::none;
}

std::string get_body_type_to_string(body_type type_value)
{
    for (const auto& [name, type] : body_type_mapping)
    {
        if (type_value == type)
            return name.data();
    }
    return "";
}

std::string get_lagrangian_to_string(lagrangian lagr)
{
    switch (lagr)
    {
    case lagrangian::l1: return "L1";
    case lagrangian::l2: return "L2";
    case lagrangian::l3: return "L3";
    case lagrangian::l4: return "L4";
    case lagrangian::l5: return "L5";
    }
    return "";
}

body_node* body_node::get_main_body()
{
    if (parent == nullptr || parent->parent == nullptr)
    {
        return this;
    }

    auto tmp = parent;
    while (tmp->parent->parent)
        tmp = tmp->parent;

    return tmp;
}

vec3d body_node::get_absolute_position()
{
    vec3d result = orbit.position;
    body_node* current_parent = parent;
    while (current_parent)
    {
        result += current_parent->orbit.position;
        current_parent = current_parent->parent;
    }
    return result;
}

vec3d body_node::get_main_body_position()
{
    vec3d result = orbit.position;
    auto main_body = get_main_body();

    if (this == main_body)
        return {};

    body_node* current_parent = parent;
    while (current_parent && current_parent != main_body)
    {
        result += current_parent->orbit.position;
        current_parent = current_parent->parent;
    }
    return result;
}

vec3d compute_l1_lagrangian(const vec3d& p1, double mass1, const vec3d& p2, double mass2)
{
    vec3d dist = p1 - p2;
    double mu = mass2 / (mass1 + mass2);
    // https://en.wikipedia.org/wiki/Lagrange_point#L1
    double L1 = dist.length() * std::cbrt(mu/3.0);

    return dist.normalized() * L1;
}

vec3d compute_l2_lagrangian(const vec3d& p1, double mass1, const vec3d& p2, double mass2)
{
    vec3d dist = p2 - p1;
    double mu = mass2 / (mass1 + mass2);
    double L2 = dist.length() * std::cbrt(mu / 3.0);

    return dist.normalized() * L2;
}

vec3d compute_l3_lagrangian(const vec3d& p1, double mass1, const vec3d& p2, double mass2)
{
    vec3d dist = p1 - p2;
    double d = dist.length();
    double mu = mass2 / (mass1 + mass2);
    // https://en.wikipedia.org/wiki/Lagrange_point#L3
    double L3 = d * 7.0 * mu / 12.0;

    return dist.normalized() * (d - L3) - p2;
}

vec3d compute_l4_lagrangian(const vec3d& p1, double mass1, const vec3d& p2, double mass2)
{
    vec3d dist = p1 - p2;
    double d = dist.length();
    vec3d midpoint = p1 + dist / 2.0;
    vec3d direction = { -dist.y, dist.x, 0.0 }; // A direction vector perpendicular to the line between p1 and p2
    double height = d * std::sqrt(3) / 2;

    return midpoint + direction.normalized() * height;
    //return { midpoint.x + height * direction.x, midpoint.y + height * direction.y, midpoint.z + height * direction.z };
}

vec3d compute_l5_lagrangian(const vec3d& p1, double mass1, const vec3d& p2, double mass2)
{
    vec3d dist = p1 - p2;
    double d = dist.length();
    vec3d midpoint = p1 + dist / 2.0;
    vec3d direction = { -dist.y, dist.x, 0.0 }; // A direction vector perpendicular to the line between p1 and p2
    double height = d * std::sqrt(3) / 2;

    return midpoint - direction.normalized() * height;
    //return { midpoint.x - height * direction.x, midpoint.y - height * direction.y, midpoint.z - height * direction.z };
}

using compute_lagr_func = vec3d(*)(const vec3d&, double, const vec3d&, double);

frame::vec3d body_node::get_lagrangian(lagrangian lagr)
{
    minor_system system(*this);

    auto compute = [&s = system](compute_lagr_func func)
    {
        return func(s.get_major_position(), s.get_major_mass(), s.get_minor_position(), s.get_minor_mass());
    };

    switch (lagr)
    {
    case lagrangian::l1: return compute(compute_l1_lagrangian);
    case lagrangian::l2: return compute(compute_l2_lagrangian);
    case lagrangian::l3: return compute(compute_l3_lagrangian);
    case lagrangian::l4: return compute(compute_l4_lagrangian);
    case lagrangian::l5: return compute(compute_l5_lagrangian);
    }

    return {};
}

static const vec3d empty_result = {};

minor_system::minor_system(body_node& minor)
    : minor(&minor)
{
    major = minor.parent;

    if (minor.system.is_major_body)
        major = minor.parent->parent;

    if (major->type == body_type::barycenter)
    {
        barycenter = major;
        major = major->system.major_body;
    }
}

const vec3d& minor_system::get_major_position()
{
    if (barycenter)
        return major->orbit.position;

    // if this is not barycentric system major body is stationary in the center
    return empty_result;
}

const vec3d& minor_system::get_major_velocity()
{
    if (barycenter)
        return major->orbit.velocity;

    // if this is not barycentric system major body is stationary in the center
    return empty_result;
}

double minor_system::get_major_mass()
{
    return major->mass;
}

kepler_orbit* minor_system::get_major_orbit()
{
    if (barycenter)
        return &major->orbit;
    return nullptr;
}

const vec3d& minor_system::get_minor_position()
{
    if (minor->system.is_major_body)
        return minor->parent->orbit.position;
    return minor->orbit.position;
}

const vec3d& minor_system::get_minor_velocity()
{
    if (minor->system.is_major_body)
        return minor->parent->orbit.velocity;
    return minor->orbit.velocity;
}

double minor_system::get_minor_mass()
{
    return minor->mass;
}

kepler_orbit& minor_system::get_minor_orbit()
{
    if (minor->system.is_major_body)
        return minor->parent->orbit;
    return minor->orbit;
}
