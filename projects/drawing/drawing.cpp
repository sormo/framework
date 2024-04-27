#include <framework.h>
#include <vector>
#include <string>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include "imgui.h"
#include "utils.h"
#include "drawing_sg.h"

frame::free_move_camera_config free_move_config;

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

    frame::draw_buffer_id rectangle;
    frame::draw_buffer_id circle;

    frame::draw_buffer_id polyline;

    frame::draw_buffer_id center;

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
    state.rectangle = frame::create_instanced_rectangle();
    state.circle = frame::create_instanced_circle(60);

    for (size_t i = 0; i < 10; i++)
        frame::add_draw_instance(state.circle, get_random_position(), 0.0f, { 10.0f, 10.0f }, get_random_color());

    for (size_t i = 0; i < 10; i++)
    {
        rect tr{ get_random_position(), frame::randf(0.0f, 6.28f), { 10.0f, 20.0f }, get_random_color() };
        tr.index = frame::add_draw_instance(state.rectangle, tr.position, tr.rotation, tr.scale, get_random_color());

        state.rects.push_back(std::move(tr));
    }

    std::vector<frame::vec2> vertices{ {0.0f, 0.0f}, {100.0f, 100.0f}, {-100.0f, 100.0f}, {0.0f, 0.0f} };

    state.polyline = frame::create_draw_buffer("polyline", { (float*)vertices.data(), vertices.size(), nullptr, 0 }, sg_primitive_type::SG_PRIMITIVETYPE_LINE_STRIP, sg_usage::SG_USAGE_DYNAMIC);
    state.center = frame::create_draw_buffer("center", frame::create_mesh_rectangle(), sg_primitive_type::SG_PRIMITIVETYPE_TRIANGLE_STRIP, sg_usage::SG_USAGE_DYNAMIC);

    //sg_range update_range;
    //update_range.ptr = state.rect.array;
    //update_range.size = sizeof(instanced_element) * state.rect.instances;

    //sg_update_buffer(state.rect.bind.vertex_buffers[1], &update_range);

    frame::set_world_transform(frame::translation(frame::get_screen_size() / 2.0f) * frame::scale({ 1.0f, -1.0f }));
    free_move_config.min_size = { 0.1f, 0.1f };
    free_move_config.boundary = frame::rectangle::from_center_size({ 400.0f, 300.0f }, { 100000.0f, 100000.0f });


}

void update()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_canvas = frame::get_world_transform().inverted().transform_point(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", frame::get_screen_size().x, frame::get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", frame::get_world_size().x, frame::get_world_size().y);

    ImGui::EndMainMenuBar();

    bool open = true;
    ImGui::SetNextWindowPos({ 0.0f, 20.0f });
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::SameLine();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    frame::draw_rectangle({}, 20.0f, 20.0f, frame::col4::RED);

    frame::free_move_camera_update(free_move_config);
}

void update_sg()
{
    for (auto& tr : state.rects)
    {
        tr.rotation += 0.5f;
        frame::update_draw_instance(state.rectangle, tr.index, tr.position, tr.rotation, tr.scale, tr.color);
    }

    frame::draw_buffer_instanced(state.rectangle);
    frame::draw_buffer_instanced(state.circle);

    frame::draw_buffer(state.polyline, {200.0f, 200.0f}, 90.0f, {0.5f, 0.5f}, frame::col4::WHITE);
    //frame::draw_buffer(state.center, frame::translation({ 0.0f, 0.0f }) * frame::scale({ 200.0f, 200.0f }), frame::col4::WHITE);
    frame::draw_buffer(state.center, frame::translation({ 0.0f, 0.0f }) * frame::scale({ 5.0f, 5.0f }), frame::col4::WHITE);
}
