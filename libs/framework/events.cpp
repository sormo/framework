#include "events.h"
#include "framework.h"
#include "imgui.h"
#include <vector>
#include <memory>
#include <cmath>
#include <set>
#include "sokol_app.h"

using namespace frame;

struct
{
    bool screen_resized = false;

    bool is_mouse_moved = false;
    vec2 mouse_delta;

    vec2 mouse_position;

    bool is_mouse_scrolled = false;
    vec2 scroll_delta;

    std::set<mouse_button> mouse_frame_pressed;
    std::set<mouse_button> mouse_frame_released;
    std::set<char> key_frame_pressed;
    std::set<char> key_frame_released;

    std::set<mouse_button> mouse_pressed;
    std::set<char> key_pressed;

} events;

namespace frame
{
    bool is_screen_resized()
    {
        return events.screen_resized;
    }

    vec2 get_mouse_screen_position()
    {
        return events.mouse_position;
    }

    bool is_mouse_down(mouse_button button)
    {
        return events.mouse_pressed.count(button);
    }

    bool is_mouse_pressed(mouse_button button)
    {
        return events.mouse_frame_pressed.count(button);
    }

    bool is_mouse_released(mouse_button button)
    {
        return events.mouse_frame_released.count(button);
    }

    bool is_mouse_scrolled()
    {
        return events.is_mouse_scrolled;
    }

    vec2 get_mouse_screen_delta()
    {
        if (events.is_mouse_moved)
            return events.mouse_delta;
        return {};
    }

    float get_mouse_wheel_delta()
    {
        if (events.is_mouse_scrolled)
            return events.scroll_delta.y;
        return {};
    }

    bool is_key_down(char key)
    {
        return events.key_pressed.count(key);
    }

    bool is_key_pressed(char key)
    {
        return events.key_frame_pressed.count(key);
    }

    bool is_key_released(char key)
    {
        return events.key_frame_released.count(key);
    }
}

void events_end_frame()
{
    events.screen_resized = false;

    events.is_mouse_moved = false;
    events.is_mouse_scrolled = false;

    events.key_frame_pressed.clear();
    events.key_frame_released.clear();

    events.mouse_frame_pressed.clear();
    events.mouse_frame_released.clear();
}

mouse_button map_sapp_mouse(sapp_mousebutton b)
{
    switch (b)
    {
    case SAPP_MOUSEBUTTON_LEFT:
        return mouse_button::left;
    case SAPP_MOUSEBUTTON_RIGHT:
        return mouse_button::right;
    case SAPP_MOUSEBUTTON_MIDDLE:
        return mouse_button::middle;
    }
    return mouse_button::left;
}

void handle_event(const sapp_event* event)
{
    switch (event->type)
    {
    case SAPP_EVENTTYPE_KEY_DOWN:
        if (event->key_repeat)
            break;
        events.key_frame_pressed.insert(event->key_code);
        events.key_pressed.insert(event->key_code);
        break;
    case SAPP_EVENTTYPE_KEY_UP:
        if (event->key_repeat)
            break;
        events.key_frame_released.insert(event->key_code);
        events.key_pressed.erase(event->key_code);
        break;
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        events.mouse_frame_pressed.insert(map_sapp_mouse(event->mouse_button));
        events.mouse_pressed.insert(map_sapp_mouse(event->mouse_button));
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        events.mouse_frame_released.insert(map_sapp_mouse(event->mouse_button));
        events.mouse_pressed.erase(map_sapp_mouse(event->mouse_button));
        break;
    case SAPP_EVENTTYPE_MOUSE_ENTER:
        events.mouse_position = { event->mouse_x, event->mouse_y };
        break;
    case SAPP_EVENTTYPE_MOUSE_MOVE:
        events.is_mouse_moved = true;
        events.mouse_delta = { event->mouse_dx, event->mouse_dy };
        events.mouse_position = { event->mouse_x, event->mouse_y };
        break;
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        events.is_mouse_scrolled = true;
        events.scroll_delta = { event->scroll_x, event->scroll_y };
        break;
    case SAPP_EVENTTYPE_RESIZED:
        events.screen_resized = true;
        break;
    }
}
