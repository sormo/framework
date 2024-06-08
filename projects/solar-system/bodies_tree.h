#pragma once
#include <kepler_orbit.h>
#include <framework.h>
#include "trajectory_resolutions.h"
#include "body.h"
#include "quadtree.h"
#include "body_info.h"
#include "camera.h"
#include <string>
#include <vector>
#include <queue>

struct body_view
{
    // set the main body (get child of Solar System barycenter)
    void set_body(body_node* body)
    {
        if (body == nullptr)
        {
            main_body = nullptr;
            body_transform = frame::identity();

            return;
        }

        main_body = body->get_main_body();

        //frame::mat3 screen_center = frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, -1.0f });
        frame::mat3 screen_center = frame::translation(frame::get_screen_size() / 2.0f) * frame::scale(frame::get_world_scale());
        body_transform = screen_center * frame::scale({ 1.0/scale_factor, 1.0/scale_factor });

        camera.follow([this, body]() { return commons::draw_cast(body->get_main_body_position() * scale_factor); });
    }

    void draw(body_color& colors)
    {
        update_current_positions();

        draw_trajectories();
        draw_points(colors);
        draw_names();
    }

    void update()
    {
        frame::save_world_transform();
        frame::set_world_transform(body_transform);
        camera.update();
        body_transform = frame::get_world_transform();
        frame::restore_world_transform();
    }

    void setup()
    {
        points_instance_buffer = frame::create_draw_buffer_instanced("points", frame::create_mesh_circle(20), SG_PRIMITIVETYPE_TRIANGLE_STRIP, SG_USAGE_DYNAMIC, 256);
    }

    body_node* main_body;
    frame::mat3 body_transform;
    camera_type camera;
    double scale_factor = 1000.0;

    frame::draw_buffer_id points_instance_buffer;

    frame::mat3 get_transform_to_world()
    {
        return frame::get_world_transform() * frame::scale({ scale_factor, scale_factor }) * frame::translation(commons::draw_cast(main_body->orbit.position));
    }

    void update_current_positions()
    {
        std::function<void(frame::vec2, body_node&)> update_recursive = [this, &update_recursive](frame::vec2 parent_position, body_node& data)
        {
            data.current_position = commons::draw_cast(data.orbit.position * scale_factor) + parent_position;

            if (!data.childs.empty())
            {
                for (auto& child : data.childs)
                    update_recursive(data.current_position, *child);
            }
        };

        main_body->current_position = {};
        for (auto& child : main_body->childs)
            update_recursive({}, *child);
    }

    void draw_points(body_color& colors)
    {
        std::function<void(body_node&, size_t&)> draw_recursive = [this, &draw_recursive, &colors](body_node& data, size_t& point_counter)
        {
            frame::vec2 position = data.current_position;

            float default_radius = commons::pixel_to_world(3.5f);
            double body_radius = commons::convert_km_to_world_size(data.radius * scale_factor);

            auto color = data.group.empty() ? colors.get(data.type) : colors.get(data.group);

            if (body_radius > default_radius)
                frame::draw_circle(position, body_radius, color);
            else
                frame::update_draw_instance(points_instance_buffer,
                    point_counter++,
                    frame::get_world_to_screen(position),
                    0.0f,
                    { 7.0f, 7.0f },
                    color);

            if (!data.childs.empty())
            {
                for (auto& child : data.childs)
                    draw_recursive(*child, point_counter);
            }
        };

        size_t point_count = 0;

        frame::save_world_transform();
        
        frame::set_world_transform(body_transform);
        draw_recursive(*main_body, point_count);

        frame::set_world_transform(frame::identity());
        frame::draw_buffer_instanced(points_instance_buffer, point_count);

        frame::restore_world_transform();
    }

    void draw_trajectories()
    {
        std::function<void(body_node&)> draw_recursive = [this, &draw_recursive](body_node& data)
        {
            data.trajectory.draw(data.orbit.semi_major_axis * scale_factor);

            if (data.childs.empty())
                return;

            frame::save_world_transform();

            frame::set_world_translation(frame::get_world_translation() + commons::draw_cast(data.orbit.position) * frame::get_world_scale());

            for (auto& child : data.childs)
                draw_recursive(*child);

            frame::restore_world_transform();
        };

        frame::save_world_transform();
        // unfortunately we need to scale up the transform because trajectories are created with tranform 1.0
        frame::set_world_transform(body_transform * frame::scale({ scale_factor, scale_factor }));
        
        // TODO not drawing the main body here

        for (auto& child : main_body->childs)
            draw_recursive(*child);

        frame::restore_world_transform();
    }

    bool is_barycenter(const body_node& body)
    {
        return body.type == body_type::barycenter;
    }

    void draw_names()
    {
        frame::save_world_transform();
        frame::set_world_transform(body_transform);

        std::vector<frame::rectangle> rectangles;
        auto check_overlap = [&rectangles](const frame::rectangle& rect)
        {
            for (const auto& rect_overlap : rectangles)
            {
                if (rect_overlap.has_overlap(rect))
                    return true;
            }
            return false;
        };

        auto get_computed_font_size = [this](const body_node* body)
        {
            double body_radius = commons::convert_km_to_world_size(body->radius * scale_factor) * frame::get_world_scale().x;
            double body_diameter = body_radius * 2.0;
            double max_char_size = body_diameter / body->name.size();

            return (float)std::min(body_diameter / 4.0, max_char_size);
        };

        auto get_text_rectangle = [](const body_node* body, const frame::vec2& position) -> frame::rectangle
        {
            auto result = body->name_text_rectangle;
            result.min /= frame::get_world_scale();
            result.max /= frame::get_world_scale();
            result.min += position;
            result.max += position;

            // the rectangle has top_left align (means [0,0] is top left) and we need bottom_left
            frame::vec2 align_modif(0.0f, result.size().y);

            std::swap(result.min.y, result.max.y);

            result.min -= align_modif;
            result.max -= align_modif;

            return result;
        };

        std::queue<body_node*> Q; // BFS

        Q.push(main_body);

        while (!Q.empty())
        {
            body_node* body = Q.front();
            Q.pop();

            frame::vec2 position = body->current_position;

            if (!is_barycenter(*body))
            {
                auto computed_font_size = get_computed_font_size(body);
                if (computed_font_size > 15.0f)
                {
                    // TODO looks like rendering text with larger font size is super slow
                    frame::draw_text_ex(body->name.c_str(), position, computed_font_size, frame::col4::LIGHTGRAY, "roboto-bold", frame::text_align::middle_middle);
                }
                else
                {
                    auto rect = get_text_rectangle(body, position);

                    if (!check_overlap(rect))
                    {
                        rectangles.push_back(std::move(rect));
                        frame::draw_text_ex(body->name.c_str(), position, 15.0f, frame::col4::LIGHTGRAY, "roboto-bold", frame::text_align::bottom_left);
                    }
                }
            }

            for (auto* child : body->childs)
                Q.push(child);
        }

        frame::restore_world_transform();
    }
};

struct bodies_tree
{
    std::vector<body_node> bodies;
    body_node* parent;

    std::vector<body_node*> query(const frame::vec2& query_point, float query_radius);
    void load(std::vector<const char*> json_datas);
    void draw(quadtree::query_result_type& parents, body_color& colors);
    void step(double time_delta);
    void clear();

private:
    void draw_names(quadtree::query_result_type& parents);
    void draw_trajectories(quadtree::query_result_type& parents, body_color& colors);
    void draw_points(quadtree::query_result_type& parents, body_color& colors);

    void update_current_positions(quadtree::query_result_type& parents);

    bool is_barycenter(const body_node& body);

    frame::draw_buffer_id points_instance_buffer;
};
