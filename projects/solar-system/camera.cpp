#include "camera.h"

void camera_type::setup()
{
    frame::vec2 world_size{ 1'000'000.0f, 1'000'000.0f };

    free_move_config.min_size = { (float)commons::MIN_ZOOMED_SIZE, (float)commons::MIN_ZOOMED_SIZE };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, world_size);
}

void camera_type::move(const frame::vec2& move_destination)
{
    move_internal([move_destination]() { return move_destination; });
}

void camera_type::follow(std::function<frame::vec2()> position)
{
    static const float max_follow_distance_px = 5.0f;

    camera_follow = position;
    camera_move = {};

    if (!position)
        return;

    auto position_world = position();
    auto move_source_world = frame::get_world_rectangle().center();

    auto distance_px = (frame::get_world_to_screen(position_world) - frame::get_world_to_screen(move_source_world)).length();

    if (distance_px > max_follow_distance_px)
        move_internal(position);
}

void camera_type::update()
{
    if (camera_move)
    {
        camera_move->update();
        if (camera_move->is_finished())
            camera_move = {};
    }
    else if (camera_follow)
    {
        auto follow_position_world = camera_follow();

        if (frame::get_mouse_wheel_delta())
        {
            frame::vec2 new_scale = frame::get_world_scale() * (1.0f + frame::get_mouse_wheel_delta() * free_move_config.zoom_speed);
            frame::set_world_scale(new_scale, follow_position_world);
        }

        auto screen_center = frame::get_screen_size() / 2.0f;

        frame::set_world_translation(screen_center, follow_position_world);
    }
    else
    {
        free_move_camera_update(free_move_config);
    }
}

void camera_type::move_internal(std::function<frame::vec2()> move_destination)
{
    static const float camera_move_speed = 1.0f;

    auto move_source = frame::get_world_rectangle().center();

    camera_move = move_data{ move_source, move_destination, commons::easing_cubic_out, camera_move_speed, free_move_config.zoom_speed };

}

camera_type::move_data::move_data(frame::vec2 world_source, std::function<frame::vec2()> world_destination, easing_func eas, float speed, float zoom_speed)
    : speed(speed), start(world_source), end(world_destination), easing(eas), zoom_speed(zoom_speed)
{
}

void camera_type::move_data::update()
{
    move_progress += frame::get_delta_time() * speed / 1000.0f;

    auto value = easing(move_progress);
    auto current = start + (end() - start) * value;

    //frame::draw_line_solid(start, end, col4::BLUE);
    //frame::draw_circle(start, commons::scale_independent(3.0f), frame::col4::RED);
    //frame::draw_circle(end, commons::scale_independent(3.0f), frame::col4::YELLOW);
    //frame::draw_circle(current, commons::scale_independent(3.0f), frame::col4::ORANGE);

    if (frame::get_mouse_wheel_delta())
    {
        frame::vec2 new_scale = frame::get_world_scale() * (1.0f + frame::get_mouse_wheel_delta() * zoom_speed); // zoom speed
        frame::set_world_scale(new_scale, current);
    }

    auto screen_center = frame::get_screen_size() / 2.0f;

    frame::set_world_translation(screen_center, current);
}

bool camera_type::move_data::is_finished()
{
    return move_progress >= 1.0f;
}
