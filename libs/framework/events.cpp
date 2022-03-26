#include "events.h"
#include "framework.h"
#include "imgui.h"
#include <vector>
#include <cmath>
#include <map>

struct MouseEvent
{
    enum class Type
    {
        Press,
        Release
    };

    int32_t Register(frame::mouse_button_callback callback, Type type, frame::mouse_button button)
    {
        if (type == Type::Press)
            callbacksPress.insert({ handleCounter++, { button, callback } });
        else
            callbacksRelease.insert({ handleCounter++, { button, callback } });

        return handleCounter - 1;
    }

    void Unregister(int32_t handle)
    {
        if (callbacksPress.count(handle))
            callbacksPress.erase(handle);
        else
            callbacksRelease.erase(handle);
    }

    void Frame()
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return;

        auto CheckButton = [this](int32_t index, bool& pressed)
        {
            if (pressed)
            {
                if (!ImGui::GetIO().MouseDown[index])
                {
                    pressed = false;
                    TriggerEventRelease((frame::mouse_button)index);
                }
            }
            else
            {
                if (ImGui::GetIO().MouseDown[index])
                {
                    pressed = true;
                    TriggerEventPress((frame::mouse_button)index);
                }
            }
        };

        CheckButton(0, isLeftPressed);
        CheckButton(1, isRightPressed);
        CheckButton(2, isMiddlePressed);
    }

private:
    void TriggerEventPress(frame::mouse_button button)
    {
        for (auto&[_, registration] : callbacksPress)
        {
            if (registration.button == button)
                registration.callback();
        }
    }

    void TriggerEventRelease(frame::mouse_button button)
    {
        for (auto& [_, registration] : callbacksRelease)
        {
            if (registration.button == button)
                registration.callback();
        }
    }

    bool isLeftPressed = false;
    bool isRightPressed = false;
    bool isMiddlePressed = false;

    int32_t handleCounter = 1;

    struct MouseRegistration
    {
        frame::mouse_button button;
        frame::mouse_button_callback callback;
    };

    std::map<int32_t, MouseRegistration> callbacksPress;
    std::map<int32_t, MouseRegistration> callbacksRelease;
};

struct ResizeEvent
{
    ResizeEvent()
    {
        width = frame::screen_width();
        height = frame::screen_height();
    }

    int32_t Register(frame::resize_callback callback)
    {
        int32_t handle = ++handleCounter;

        callbacks.insert({ handle, callback });
        return handle;
    }

    void Unregister(int32_t handle)
    {
        callbacks.erase(handle);
    }

    void Frame()
    {
        if (std::fabsf(width - frame::screen_width()) > 1.0f || std::fabsf(height - frame::screen_height()) > 1.0f)
        {
            for (auto&[_, callback] : callbacks)
                callback();

            width = frame::screen_width();
            height = frame::screen_height();
        }
    }

    float width;
    float height;

    int32_t handleCounter = 0;
    std::map<int32_t, frame::resize_callback> callbacks;
};

MouseEvent mouse_event;
std::unique_ptr<ResizeEvent> resize_event;

namespace frame
{
    Point mouse_screen_position()
    {
        return Point{ ImGui::GetIO().MousePos.x, frame::screen_height() - ImGui::GetIO().MousePos.y };
    }

    Point mouse_canvas_position()
    {
        return convert_from_screen_to_canvas(mouse_screen_position());
    }

    bool mouse_pressed(mouse_button button)
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return false;
        return ImGui::GetIO().MouseDown[(int32_t)button];
    }

    int32_t mouse_press_register(mouse_button button, mouse_button_callback callback)
    {
        return mouse_event.Register(callback, MouseEvent::Type::Press, button);
    }

    int32_t mouse_release_register(mouse_button button, mouse_button_callback callback)
    {
        return mouse_event.Register(callback, MouseEvent::Type::Release, button);
    }

    void mouse_unregister(int32_t handle)
    {
        mouse_event.Unregister(handle);
    }

    int32_t resize_register(resize_callback callback)
    {
        if (!resize_event)
            resize_event = std::make_unique<ResizeEvent>();
        return resize_event->Register(callback);
    }

    void resize_unregister(int32_t handle)
    {
        if (!resize_event)
            resize_event = std::make_unique<ResizeEvent>();
        resize_event->Unregister(handle);
    }

    Point mouse_screen_delta()
    {
        return { -ImGui::GetIO().MouseDelta.x, ImGui::GetIO().MouseDelta.y };
    }

    float mouse_wheel_delta()
    {
        return ImGui::GetIO().MouseWheel;
    }
}

void events_frame()
{
    mouse_event.Frame();
    if (resize_event)
        resize_event->Frame();
}
