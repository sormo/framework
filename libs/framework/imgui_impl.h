#pragma once
#include <cstdlib>

struct sapp_event;

namespace imgui
{
    void setup(void* fontData, size_t fontSize);
    void prepare_render();
    void render();
    bool handle_event(const sapp_event* event);
}
