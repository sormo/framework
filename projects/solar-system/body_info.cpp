#include "body_info.h"
#include <json.hpp>
#include <framework.h>
#include <fstream>
#include "svg.h"
#include "zip.h"

using namespace frame;

// TODO move
// https://stackoverflow.com/questions/24716250/c-store-read-binary-file-into-buffer
static std::vector<char> read_binary_file(const std::string filename)
{
    // binary mode is only for switching off newline translation
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);

    std::streampos file_size;
    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> vec;
    vec.reserve(file_size);
    vec.insert(vec.begin(), std::istream_iterator<char>(file), std::istream_iterator<char>());
    return (vec);
}

void body_info::setup(const std::vector<char>& body_icons_zip)
{
    auto zip = frame::open_zip(body_icons_zip);

    auto zip_files = frame::list_zip_files(*zip);
    for (const auto& file : zip_files)
    {
        auto data = frame::get_zip_file(*zip, file);
        icons[file] = frame::svg_parse(data.data());
    }
}

void body_info::set_body(body_node* body)
{
    this->body = body;
    icon = nullptr;

    if (body)
    {
        // TODO this is ugly
        for (const auto& [name, icon] : icons)
        {
            auto body_icon_name = name.substr(0, name.find('.'));
            auto body_name_lower_case = make_lower_case_string(body->name);

            if (body_name_lower_case.find(body_icon_name) != std::string::npos)
            {
                this->icon = icon;
                break;
            }
        }

        if (!icon)
        {
            if (body->type == body_type::comet)
                icon = icons["comet.svg"];
            else if (body->type == body_type::minor_planet)
                icon = icons["asteroid.svg"];
            else if (body->type == body_type::moon)
                icon = icons["moon_decrescent.svg"];
        }
    }
}

void body_info::draw(body_color& colors)
{
    static const vec2 offset(20.0f, 20.0f);
    static const vec2 size(300.0f, 300.0f);

    if (body == nullptr)
        return;

    frame::save_world_transform();
    frame::set_world_transform(frame::mat3::identity());
    frame::set_world_translation(offset);

    //frame::draw_rectangle(frame::rectangle::from_min_max({}, size), col4::RGBf(0.0f, 0.0f, 0.0f, 0.65f));
    frame::draw_rounded_rectangle(size/2.0f, size.x, size.y, 10.0f, col4::RGBf(0.0f, 0.0f, 0.0f, 0.65f));

    // constants
    static const float boundary = 10.0f;

    // draw name, type and group
    static const float name_size = 30.0f;
    static const float type_size = 20.0f;
    static const float group_size = 13.0f;

    if (icon)
    {
        frame::svg_draw_ex_size(icon, { boundary, boundary }, 0.0f, { name_size - 6.0f , name_size - 6.0f });
    }

    frame::draw_text_ex(body->name.c_str(), { boundary + name_size, boundary }, name_size, col4::LIGHTGRAY, "roboto-black", frame::text_align::top_left);
    frame::draw_text_ex(get_body_type_to_string(body->type).c_str(), { boundary, boundary + name_size - 2.0f }, type_size, colors.get(body->type), "roboto-bold", frame::text_align::top_left);
    frame::draw_text_ex(body->group.c_str(), { size.x - boundary, boundary + name_size + 2.0f }, group_size, colors.get(body->group), "roboto-bold", frame::text_align::top_right);

    // draw properties
    static const float property_size = 15.0f;
    static const float column_name_size = 150.0f;
    static const float column_value_size = 80.0f;

    auto draw_property_str = [](float& y, const char* name, const char* value, const char* unit)
    {
        frame::draw_text_ex(name, vec2(boundary, y), property_size, col4::LIGHTGRAY, "roboto-medium");
        frame::draw_text_ex(value, vec2(boundary + column_name_size, y), property_size, col4::LIGHTGRAY, "roboto-medium");
        frame::draw_text_ex(unit, vec2(boundary + column_name_size + column_value_size, y), property_size, col4::LIGHTGRAY, "roboto-medium");
        y += property_size + 0.5f * property_size;
    };

    auto draw_property_num = [&draw_property_str](float& y, const char* name, double value, const char* unit)
    {
        draw_property_str(y, name, commons::convert_double_to_string(value).c_str(), unit);
    };

    auto draw_separator = [&](float& y, const char* name)
    {
        frame::draw_text_ex(name, { boundary, y }, property_size, colors.get(body->type), "roboto-medium");
        frame::draw_line_solid_ex({ boundary, y + property_size - 1.0f }, { boundary + size.x - 2.0f * boundary, y + property_size - 1.0f }, 2.0f, colors.get(body->type));
        frame::draw_line_solid_ex({ boundary, y + property_size + 1.0f }, { boundary + size.x - 2.0f * boundary, y + property_size - 1.0f }, 3.0f, colors.get(body->group));
        y += property_size + 0.5f * property_size + 3.0f;
    };

    float y_value = name_size + 2.0f*type_size + group_size;

    // physical properties
    draw_separator(y_value, "Physical Properties:");

    draw_property_num(y_value, "Mean Radius", body->radius, "km");

    if (body->mass != 0.0)
        draw_property_num(y_value, "Mass", body->mass, "kg");

    if (body->density != 0.0)
        draw_property_num(y_value, "Density", body->density, "g/cm^3");

    if (!body->dimensions_str.empty())
        draw_property_str(y_value, "Dimensions", body->dimensions_str.c_str(), "km");

    // orbit properties
    draw_separator(y_value, "Orbit Properties:");

    body_node* orbit_node = body;
    if (body->is_major_body && body->parent->type == body_type::barycenter)
        orbit_node = body->parent;

    draw_property_num(y_value, "Period", body->period, "days");
    draw_property_num(y_value, "Semi-major Axis", body->orbit.semi_major_axis, "AU");
    draw_property_num(y_value, "Eccentricity", body->orbit.eccentricity, "");
    draw_property_num(y_value, "Inclination", rad_to_deg(body->inclination), "deg");

    frame::restore_world_transform();
}

void body_color::setup(const char* colors_json)
{
    nlohmann::json data = nlohmann::json::parse(colors_json);

    for (const auto& body_type : data["body_type"].items())
        type_colors[get_body_type_from_string(body_type.key())] = col4::HEXs(std::string{ body_type.value() });

    for (const auto& group : data["body_group"].items())
        group_colors[group.key()] = col4::HEXs(std::string{ group.value() });
}

col4 body_color::get(body_type type)
{
    return type_colors[type];
}

col4 body_color::get(const std::string& group)
{
    return group_colors[group];
}
