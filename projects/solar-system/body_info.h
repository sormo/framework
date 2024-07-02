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
    std::unordered_map<body_type, frame::col4> type_colors;
    std::unordered_map<std::string, frame::col4> group_colors;
};

struct body_info
{
    void draw();

    void set_body(body_node* body);
    body_node* get_body();

    void setup(const std::vector<char>& body_icons_zip, body_color& cols);

    bool accept_click(const frame::vec2& screen_position);

private:
    float draw_internal(bool skip_draw = false);

    void draw_name_type_and_group(float& y, bool skip_draw = false);
    void draw_property_str(float& y, const char* name, const char* value, const char* unit, bool skip_draw = false);
    void draw_property_num(float& y, const char* name, double value, const char* unit, bool skip_draw = false);
    void draw_separator(float& y, const char* name, bool skip_draw = false);

    frame::rectangle get_info_rectangle();

    body_node* body = nullptr;
    frame::vec2 draw_size;
    frame::vec2 draw_position; // upper-left corner
    frame::svg_image* icon = nullptr;
    body_color* colors = nullptr;

    bool is_collapsed = true;

    std::unordered_map<std::string, frame::svg_image*> icons;

    frame::svg_image* expand_icon = nullptr;
};
