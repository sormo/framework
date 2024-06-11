#include "body.h"

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
