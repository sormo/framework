#include "body.h"

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
