#include "body_info.h"
#include "commons.h"
#include <json.hpp>
#include <framework.h>
#include <fstream>
#include "svg.h"
#include "zip.h"

using namespace frame;

// constants
static const float boundary = 10.0f;
static const float x_size = 300.0f;

// draw name, type and group
static const float name_y_size = 30.0f;
static const float type_y_size = 20.0f;
static const float group_y_size = 13.0f;
static const float icon_size = 30.0f;

// draw properties
static const float property_y_size = 15.0f;
static const float property_column_name_x_size = 150.0f;
static const float property_column_value_x_size = 80.0f;

static const float y_size_collapsed = name_y_size + type_y_size + boundary;

void body_info::draw_name_type_and_group(float& y, bool skip_draw)
{
    if (!skip_draw)
    {
        if (icon)
        {
            frame::draw_svg_ex_size(icon, { boundary, y }, 0.0f, { icon_size - 6.0f , icon_size - 6.0f });
        }

        frame::draw_text_ex(body->name.c_str(),
                           { boundary + icon_size, y },
                           name_y_size,
                           col4::LIGHTGRAY,
                           "roboto-black",
                           frame::text_align::top_left);

        frame::draw_text_ex(get_body_type_to_string(body->type).c_str(),
                           { boundary, y + name_y_size - 2.0f },
                           type_y_size,
                           colors->get(body->type),
                           "roboto-bold",
                           frame::text_align::top_left);

        frame::draw_text_ex(body->group.c_str(),
                           { draw_size.x - boundary, y + name_y_size + 2.0f },
                           group_y_size, colors->get(body->group),
                           "roboto-bold",
                           frame::text_align::top_right);

        float collapsed_icon_angle = is_collapsed ? frame::deg_to_rad(90.0f) : 0.0f;

        frame::draw_svg_ex(expand_icon, { draw_size.x - boundary, y }, collapsed_icon_angle, {0.7f, 0.7f}, frame::text_align::top_right);
    }

    y += name_y_size + type_y_size + boundary;
}

void body_info::draw_property_str(float& y, const char* name, const char* value, const char* unit, bool skip_draw)
{
    if (!skip_draw)
    {
        frame::draw_text_ex(name, vec2(boundary, y), property_y_size - 1.0f, col4::LIGHTGRAY, "roboto");
        frame::draw_text_ex(value, vec2(boundary + property_column_name_x_size, y), property_y_size - 3.0f, col4::LIGHTGRAY, "roboto");
        frame::draw_text_ex(unit, vec2(boundary + property_column_name_x_size + property_column_value_x_size, y), property_y_size - 3.0f, col4::LIGHTGRAY, "roboto");
    }
    y += property_y_size + 0.5f * property_y_size;
}

void body_info::draw_property_num(float& y, const char* name, double value, const char* unit, bool skip_draw)
{
    draw_property_str(y, name, commons::convert_double_to_string(value).c_str(), unit);
}

void body_info::draw_separator(float& y, const char* name, bool skip_draw)
{
    if (!skip_draw)
    {
        frame::draw_text_ex(name, { boundary, y }, property_y_size, col4::LIGHTGRAY, "roboto-medium");
        frame::draw_line_solid_ex({ 0.0f, y + property_y_size }, { draw_size.x, y + property_y_size }, 1.0f, col4::RGB(89, 88, 87));
    }
    y += property_y_size + 0.5f * property_y_size + 3.0f;
}

void body_info::setup(const std::vector<char>& body_icons_zip, body_color& cols)
{
    colors = &cols;

    auto zip = frame::open_zip(body_icons_zip);

    auto zip_files = frame::list_zip_files(*zip);
    for (const auto& file : zip_files)
    {
        auto data = frame::get_zip_file(*zip, file);
        icons[file] = frame::svg_parse(data.data());
    }

    static const char expand_icon_src[] = R"(<svg width="20" height="20" version="1.1" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg"">
                                              <path d="m19 6.25-1.5-1.5-7.5 7.5-7.5-7.5-1.5 1.5 9 9 9-9z" fill="#c8c8c8" stroke-opacity="0" style="paint-order:fill markers stroke"/>
                                             </svg>)";

    expand_icon = frame::svg_parse(expand_icon_src);
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
            else if (body->type == body_type::spacecraft)
                icon = icons["spacecraft.svg"];
        }

        auto y_size = draw_internal(true);

        draw_size = vec2(300.0f, y_size);
        draw_position = { 10.0f, 10.0f };
    }
}

body_node* body_info::get_body()
{
    return body;
}

static std::pair<double, const char*> get_period_value_and_unit(double period)
{
    if (period < 1.0)
        return { period * 24.0, "hours" };
    if (period < 10'000)
        return { period, "days" };
    return { period / 365.0, "years" };
}

static std::pair<double, const char*> get_semi_major_axis_value_and_unit(double semi_major_axis)
{
    if (semi_major_axis > 0.01)
        return { semi_major_axis, "AU" };
    return { semi_major_axis * 1.496e8, "km" };
}

static std::pair<double, const char*> get_mean_radius_value_and_unit(double radius)
{
    if (radius < 1.0)
        return { radius * 1000.0, "m" };
    return { radius, "km" };
}

bool body_info::accept_click(const frame::vec2& screen_position)
{
    if (!body)
        return false;

    if (get_info_rectangle().contains(screen_position))
    {
        is_collapsed = !is_collapsed;
        return true;
    }
    return false;
}

float body_info::draw_internal(bool skip_draw)
{
    float y_value = boundary;

    draw_name_type_and_group(y_value, skip_draw);

    // skip_draw is not best name, it is used to measure the height of info
    if (is_collapsed && !skip_draw)
        return y_size_collapsed;

    draw_separator(y_value, "Physical Properties:", skip_draw);

    auto [radius_value, radius_unit] = get_mean_radius_value_and_unit(body->radius);
    draw_property_num(y_value, "Mean Radius", radius_value, radius_unit, skip_draw);

    if (body->mass != 0.0)
        draw_property_num(y_value, "Mass", body->mass, "kg", skip_draw);

    if (body->density != 0.0)
        draw_property_num(y_value, "Density", body->density, "g/cm^3", skip_draw);

    if (!body->dimensions_str.empty())
        draw_property_str(y_value, "Dimensions", body->dimensions_str.c_str(), "km", skip_draw);

    y_value += 3.0f;

    // orbit properties
    draw_separator(y_value, "Orbit Properties:", skip_draw);

    body_node* orbit_node = body;
    if (body->system.is_major_body && body->parent->type == body_type::barycenter)
        orbit_node = body->parent;

    auto [period_value, period_unit] = get_period_value_and_unit(orbit_node->period);
    auto [semi_major_axis_value, semi_major_axis_unit] = get_semi_major_axis_value_and_unit(orbit_node->orbit.semi_major_axis);

    draw_property_num(y_value, "Period", period_value, period_unit, skip_draw);
    draw_property_num(y_value, "Semi-major Axis", semi_major_axis_value, semi_major_axis_unit, skip_draw);
    draw_property_num(y_value, "Eccentricity", orbit_node->orbit.eccentricity, "", skip_draw);
    draw_property_num(y_value, "Inclination", rad_to_deg(orbit_node->inclination), "deg", skip_draw);

    return y_value;
}

frame::rectangle body_info::get_info_rectangle()
{
    vec2 info_size = is_collapsed ? vec2{ draw_size.x, y_size_collapsed } : draw_size;

    return rectangle::from_center_size(info_size / 2.0f, info_size);
}

void body_info::draw()
{
    if (body == nullptr)
        return;

    frame::save_world_transform();
    frame::set_world_transform(frame::mat3::identity());
    frame::set_world_translation(draw_position);

    auto rect = get_info_rectangle();

    frame::draw_rounded_rectangle(rect.center(), rect.size().x, rect.size().y, 7.0f, col4::RGB(41, 40, 38));
    frame::draw_rounded_rectangle_ex(rect.center(), 0.0f, rect.size().x, rect.size().y, 7.0f, col4::BLANK, 1.0f, col4::RGB(89,88,87));

    //frame::set_world_translation(draw_position + vec2{ boundary, boundary });

    draw_internal();

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
