#pragma once
#include "utils.h"

struct camera_type
{
    void setup()
    {
        frame::vec2 world_size{ 1'000'000.0f, 1'000'000.0f };

        free_move_config.min_size = { (float)commons::MIN_ZOOMED_SIZE, (float)commons::MIN_ZOOMED_SIZE };
        free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, world_size);
    }

    void update()
    {
        static const float camera_move_speed = 1.0f;

        if (frame::is_mouse_released(frame::mouse_button::left))
        {
            auto move_source = frame::get_world_rectangle().center();
            auto move_destination = frame::get_mouse_world_position();
            
            camera_move = move_data{ move_source, move_destination, commons::easing_cubic_out, camera_move_speed, free_move_config.zoom_speed };
        }
        else if (camera_move)
        {
            camera_move->update();
            if (camera_move->is_finished())
                camera_move = {};
        }
        else
        {
            free_move_camera_update(free_move_config);
        }
    }

private:

    struct move_data
    {
        using easing_func = float(float);

        move_data(frame::vec2 world_source, frame::vec2 world_destination, easing_func eas, float speed = 1.0f, float zoom_speed = 0.01f)
            : speed(speed), start(world_source), end(world_destination), easing(eas), zoom_speed(zoom_speed)
        {
        }

        void update()
        {
            move_progress += frame::get_delta_time() * speed / 1000.0f;

            auto value = easing(move_progress);
            auto current = start + (end - start) * value;

            //frame::draw_line_solid(start, end, col4::BLUE);
            //frame::draw_circle(start, commons::scale_independent(3.0f), frame::col4::RED);
            //frame::draw_circle(end, commons::scale_independent(3.0f), frame::col4::YELLOW);
            //frame::draw_circle(current, commons::scale_independent(3.0f), frame::col4::ORANGE);

            if (frame::get_mouse_wheel_delta())
            {
                frame::vec2 new_scale = frame::get_world_transform().get_scale() * (1.0f + frame::get_mouse_wheel_delta() * zoom_speed); // zoom speed
                frame::set_world_scale(new_scale, current);
            }

            auto screen_center = frame::get_screen_size() / 2.0f;

            frame::set_world_translation(screen_center, current);
        }

        bool is_finished()
        {
            return move_progress >= 1.0f;
        }

        std::function<float(float)> easing;
        frame::vec2 start;
        frame::vec2 end;
        float speed;
        float zoom_speed;
        float move_progress = 0.0f;
    };

    frame::free_move_camera_config free_move_config;
    std::optional<move_data> camera_move;
};
