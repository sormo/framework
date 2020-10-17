#pragma once
#include <cstdlib>

struct sapp_event;

namespace imgui
{
    void setup(void* fontData, size_t fontSize);
    void prepare_render(float width, float height);
    void render();
    void handle_event(const sapp_event* event);
}
