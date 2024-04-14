#include <framework.h>
#include <vector>
#include <string>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include "imgui.h"
#include "utils.h"
#include "drawing_sg.h"

struct rect
{
    frame::vec2 position;
    float rotation;
    frame::vec2 scale;
    frame::col4 color;
    size_t index;
};

struct
{
    std::vector<rect> rects;

    frame::instanced_id rectangle;
    frame::instanced_id circle;

} state;

frame::col4 get_random_color()
{
    return { frame::randf(), frame::randf(), frame::randf(), 1.0f };
}

frame::vec2 get_random_position()
{
    return { frame::randf(-sapp_widthf()/2.0f, sapp_widthf() / 2.0f), frame::randf(-sapp_heightf() / 2.0f, sapp_heightf() / 2.0f)};
}

void setup()
{
    frame::setup_instanced();

    //state.rectangle = frame::create_instanced_rectangle();
    state.circle = frame::create_instanced_circle(60);

    for (size_t i = 0; i < 1000; i++)
        frame::add_instanced(state.circle, get_random_position(), 0.0f, { 10.0f, 10.0f }, get_random_color());

    //for (size_t i = 0; i < 500; i++)
    //{
    //    rect tr{ get_random_position(), frame::randf(0.0f, 6.28f), {10.0f, 20.0f}, get_random_color() };
    //    tr.index = frame::add_instanced(state.rectangle, tr.position, tr.rotation, tr.scale, get_random_color());

    //    state.rects.push_back(std::move(tr));
    //}

    //sg_range update_range;
    //update_range.ptr = state.rect.array;
    //update_range.size = sizeof(instanced_element) * state.rect.instances;

    //sg_update_buffer(state.rect.bind.vertex_buffers[1], &update_range);
}

void update()
{
    bool open = true;
    ImGui::SetNextWindowPos({ 10.0f, 10.0f });
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void update_sg()
{
    for (auto& tr : state.rects)
    {
        tr.rotation += 0.5f;
        frame::update_instanced(state.rectangle, tr.index, tr.position, tr.rotation, tr.scale, tr.color);
    }

    //frame::draw_instanced(state.rectangle);

    frame::draw_instanced(state.circle);
}
