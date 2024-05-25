#pragma once
#include "body.h"
#include <svg.h>
#include <map>
#include <string>

struct body_color
{
    void setup(const char* colors_json);

    frame::col4 get(body_type type);
    frame::col4 get(const std::string& group);

private:
    std::map<body_type, frame::col4> type_colors;
    std::map<std::string, frame::col4> group_colors;
};

struct body_info
{
    void draw();

    void set_body(body_node* body);

    void setup(const std::vector<char>& body_icons_zip, body_color& cols);

private:
    float draw_internal(bool skip_draw = false);

    void draw_name_type_and_group(float& y, bool skip_draw = false);
    void draw_property_str(float& y, const char* name, const char* value, const char* unit, bool skip_draw = false);
    void draw_property_num(float& y, const char* name, double value, const char* unit, bool skip_draw = false);
    void draw_separator(float& y, const char* name, bool skip_draw = false);

    body_node* body = nullptr;
    frame::vec2 draw_size;
    frame::vec2 draw_position; // upper-left corner
    frame::svg_image* icon;
    body_color* colors = nullptr;

    std::map<std::string, frame::svg_image*> icons;
};
