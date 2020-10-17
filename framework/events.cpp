#include "events.h"
#include "framework.h"
#include "imgui.h"
#include <vector>
#include <cmath>
#include <unordered_map>

struct MouseEvent
{
    enum class Type
    {
        Press,
        Release
    }
    type;

    MouseEvent(Type t)
        : type(t)
    {
    }

    int32_t Register(frame::mouse_callback callback)
    {
        callbacks.push_back(callback);
        return handleCounter++;
    }

    void Unregister(int32_t handle)
    {
        callbacks[handle] = nullptr;
    }

    void Frame()
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return;

        // press event
        if (ImGui::GetIO().MouseDown[0] && !isPressed)
        {
            isPressed = true;
            if (type == Type::Press)
                TriggerEvent();
        }

        // release event
        if (!ImGui::GetIO().MouseDown[0] && isPressed)
        {
            isPressed = false;
            if (type == Type::Release)
                TriggerEvent();
        }
    }

private:
    void TriggerEvent()
    {
        for (auto& callback : callbacks)
        {
            if (callback)
                callback();
        }
    }

    bool isPressed = false;

    int32_t handleCounter = 0;
    std::vector<frame::mouse_callback> callbacks;
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
            width = frame::screen_width();
            height = frame::screen_height();

            for (auto&[_, callback] : callbacks)
                callback(width, height);
        }
    }

    float width;
    float height;

    int32_t handleCounter = 0;
    std::unordered_map<int32_t, frame::resize_callback> callbacks;
};

MouseEvent pressEvent(MouseEvent::Type::Press);
MouseEvent releaseEvent(MouseEvent::Type::Release);

std::unique_ptr<ResizeEvent> resizeEvent;

namespace frame
{
    Point mouse_screen_position()
    {
        return { ImGui::GetIO().MousePos.x, frame::screen_height() - ImGui::GetIO().MousePos.y };
    }

    bool mouse_pressed()
    {
        if (ImGui::GetIO().WantCaptureMouse)
            return false;

        // left mouse button
        return ImGui::GetIO().MouseDown[0];
    }

    int32_t mouse_press_register(mouse_callback callback)
    {
        return pressEvent.Register(callback);
    }

    void mouse_press_unregister(int32_t handle)
    {
        return pressEvent.Unregister(handle);
    }

    int32_t mouse_release_register(mouse_callback callback)
    {
        return releaseEvent.Register(callback);
    }

    void mouse_release_unregister(int32_t handle)
    {
        return releaseEvent.Unregister(handle);
    }

    int32_t resize_register(resize_callback callback)
    {
        if (!resizeEvent)
            resizeEvent = std::make_unique<ResizeEvent>();
        return resizeEvent->Register(callback);
    }

    void resize_unregister(int32_t handle)
    {
        if (!resizeEvent)
            resizeEvent = std::make_unique<ResizeEvent>();
        resizeEvent->Unregister(handle);
    }
}

void events_frame()
{
    pressEvent.Frame();
    releaseEvent.Frame();
    if (resizeEvent)
        resizeEvent->Frame();
}
