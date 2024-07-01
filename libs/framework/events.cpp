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

    touch_id touches_frame_moved[SAPP_MAX_TOUCHPOINTS] = {};
    vec2 touches_frame_moved_delta[SAPP_MAX_TOUCHPOINTS] = {};
    size_t touches_frame_moved_count = 0;

    std::set<mouse_button> mouse_frame_pressed;
    std::set<mouse_button> mouse_frame_released;
    std::set<char> key_frame_pressed;
    std::set<char> key_frame_released;
    std::vector<touch_id> touches_frame_pressed;
    std::vector<touch_id> touches_frame_released;

    std::set<mouse_button> mouse_down;
    std::set<char> key_down;
    std::vector<touch_id> touches_down;
    std::unordered_map<touch_id, vec2> touches_down_positions;

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
        return events.mouse_down.count(button);
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
        return events.key_down.count(key);
    }

    bool is_key_pressed(char key)
    {
        return events.key_frame_pressed.count(key);
    }

    bool is_key_released(char key)
    {
        return events.key_frame_released.count(key);
    }

    const std::vector<touch_id>& get_touches_pressed()
    {
        return events.touches_frame_pressed;
    }

    const std::vector<touch_id>& get_touches_released()
    {
        return events.touches_frame_released;
    }

    const std::vector<touch_id>& get_touches_down()
    {
        return events.touches_down;
    }

    bool is_touch_down(touch_id id)
    {
        return std::find(std::begin(events.touches_down), std::end(events.touches_down), id) != std::end(events.touches_down);
    }

    vec2 get_touch_screen_position(touch_id id)
    {
        if (events.touches_down_positions.count(id) == 0)
            return {};
        return events.touches_down_positions[id];
    }

    vec2 get_touch_screen_delta(touch_id id)
    {
        for (size_t i = 0; i < events.touches_frame_moved_count; i++)
        {
            if (events.touches_frame_moved[i] == id)
                return events.touches_frame_moved_delta[i];
        }
        return {};
    }

    static std::vector<std::pair<touch_id, vec2>> debug_touches;
    sapp_event create_debug_touch_event(sapp_event_type type, touch_id changed_id)
    {
        sapp_event event = {};
        event.type = type;
        event.num_touches = (int)debug_touches.size();
        for (size_t i = 0; i < debug_touches.size(); i++)
        {
            event.touches[i].identifier = debug_touches[i].first;
            event.touches[i].changed = debug_touches[i].first == changed_id;
            event.touches[i].pos_x = debug_touches[i].second.x;
            event.touches[i].pos_y = debug_touches[i].second.y;
        }
        return event;
    }

    void set_debug_touch_down(touch_id id, vec2 screen_position)
    {
        debug_touches.push_back({ id, screen_position });

        auto event = create_debug_touch_event(SAPP_EVENTTYPE_TOUCHES_BEGAN, id);
        handle_event(&event);
    }

    void set_debug_touch_move(touch_id id, vec2 screen_position)
    {
        for (size_t i = 0; i < debug_touches.size(); i++)
        {
            if (debug_touches[i].first == id)
                debug_touches[i].second = screen_position;
        }

        auto event = create_debug_touch_event(SAPP_EVENTTYPE_TOUCHES_MOVED, id);
        handle_event(&event);
    }

    void set_debug_touch_up(touch_id id)
    {
        auto event = create_debug_touch_event(SAPP_EVENTTYPE_TOUCHES_ENDED, id);
        handle_event(&event);

        debug_touches.erase(std::remove_if(debug_touches.begin(), debug_touches.end(), [id](const auto& o) {return o.first == id; }), debug_touches.end());
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

    events.touches_frame_pressed.clear();

    // positions for release touches are removed at the end of frame
    for (size_t i = 0; i < events.touches_frame_released.size(); i++)
        events.touches_down_positions.erase(events.touches_frame_released[i]);
    events.touches_frame_released.clear();

    events.touches_frame_moved_count = 0;
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

void handle_touches_began(const sapp_touchpoint touches[SAPP_MAX_TOUCHPOINTS], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (touches[i].changed)
        {
            events.touches_frame_pressed.push_back(touches[i].identifier);
            events.touches_down.push_back(touches[i].identifier);
            events.touches_down_positions.insert({ (touch_id)touches[i].identifier, {touches[i].pos_x, touches[i].pos_y} });
        }
    }
}

void handle_touches_ended(const sapp_touchpoint touches[SAPP_MAX_TOUCHPOINTS], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (touches[i].changed)
        {
            events.touches_frame_released.push_back(touches[i].identifier);
            events.touches_down.erase(std::remove(events.touches_down.begin(), events.touches_down.end(), touches[i].identifier), events.touches_down.end());
        }
    }
}

void handle_touches_moved(const sapp_touchpoint touches[SAPP_MAX_TOUCHPOINTS], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        if (touches[i].changed)
        {
            const sapp_touchpoint& touch = touches[i];
            auto index = events.touches_frame_moved_count++;
            auto position = vec2(touch.pos_x, touch.pos_y);

            events.touches_frame_moved[index] = touch.identifier;
            events.touches_frame_moved_delta[index] = position - events.touches_down_positions[touch.identifier];
            events.touches_down_positions[touch.identifier] = position;
        }
    }
}

void handle_event(const sapp_event* event)
{
    switch (event->type)
    {
    case SAPP_EVENTTYPE_KEY_DOWN:
        if (event->key_repeat)
            break;
        events.key_frame_pressed.insert(event->key_code);
        events.key_down.insert(event->key_code);
        break;
    case SAPP_EVENTTYPE_KEY_UP:
        if (event->key_repeat)
            break;
        events.key_frame_released.insert(event->key_code);
        events.key_down.erase(event->key_code);
        break;
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        events.mouse_frame_pressed.insert(map_sapp_mouse(event->mouse_button));
        events.mouse_down.insert(map_sapp_mouse(event->mouse_button));
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        events.mouse_frame_released.insert(map_sapp_mouse(event->mouse_button));
        events.mouse_down.erase(map_sapp_mouse(event->mouse_button));
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
    case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        handle_touches_began(event->touches, event->num_touches);
        break;
    case SAPP_EVENTTYPE_TOUCHES_MOVED:
        handle_touches_moved(event->touches, event->num_touches);
        break;
    case SAPP_EVENTTYPE_TOUCHES_ENDED:
    case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
        handle_touches_ended(event->touches, event->num_touches);
        break;
    }
}
