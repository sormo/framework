#pragma once
#include "body.h"
#include <svg.h>
#include <map>
#include <string>

struct body_color
{
    void setup();

    frame::col4 get(body_type type);
    frame::col4 get(const std::string& group);

private:
    std::map<body_type, frame::col4> type_colors;
    std::map<std::string, frame::col4> group_colors;
};

struct body_info
{
    void draw(body_color& colors);

    void set_body(body_node* body);

    void setup();

private:
    body_node* body = nullptr;
    frame::svg_image* icon;
    std::map<std::string, frame::svg_image*> icons;
};
