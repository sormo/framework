#pragma once
#include "utils.h"
#include "commons.h"

struct camera_type
{
    void setup();

    void move(const frame::vec2& move_destination);

    // move_if_needed means that we will move center of camera to position if needed
    // if set to false, camera is set to position directly
    void follow(std::function<frame::vec2()> position, bool move_if_needed = true);

    void update();

    frame::free_move_camera_config free_move_config;

private:

    void move_internal(std::function<frame::vec2()> move_destination);

    static void scale_internal(const frame::vec2& scale_position, float zoom_speed);

    struct move_data
    {
        using easing_func = float(float);

        move_data(frame::vec2 world_source, std::function<frame::vec2()> world_destination, easing_func eas, float speed = 1.0f, float zoom_speed = 0.01f);

        void update();

        bool is_finished();

        std::function<float(float)> easing;
        frame::vec2 start;
        std::function<frame::vec2()> end;
        float speed;
        float zoom_speed;
        float move_progress = 0.0f;
    };

    std::optional<move_data> camera_move;
    std::function<frame::vec2()> camera_follow;
};
