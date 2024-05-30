#include <framework.h>
#include <vector>
#include <string>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_time.h>
#include "imgui.h"
#include "utils.h"
#include "drawing_sg.h"
#include "svg.h"

using namespace frame;

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

frame::svg_image* svg_test;
frame::image image_test;

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

    char earth_svg[] = R"(<?xml version="1.0" encoding="UTF-8"?>
                          <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 12 12" version="1.1" style="fill:none;stroke-width:0.6;stroke:#ffffff">
	                          <circle cx="6" cy="6" r="5"/>
	                          <path d="M 6 1 L 6 11" />
	                          <path d="M 1 6 L 11 6" />
                          </svg>)";

    char sun_svg[] = R"(<?xml version="1.0" encoding="UTF-8"?>
                        <svg viewBox="0 0 50 50" xmlns="http://www.w3.org/2000/svg" style="fill:#ffffff>
                         <g stroke-width="none">
                          <title>U+1F31E SUN WITH FACE</title>
                          <path d="m25 8q3.2483-1.7203 2.0584-3.8989-1.4939-2.0942 2.8189-3.6208-0.39018 1.9616 1.2187 4.4239 1.175 2.542 0.40964 4.3898 3.6593-0.34627 3.3937-2.8144-0.57878-2.5065 3.9899-2.2664-1.1111 1.6629-0.567 4.5535 0.11277 2.7982-1.3014 4.2124 3.5133 1.0804 4.2124-1.3014 0.42446-2.5372 4.5535-0.567-1.6629 1.1111-2.2664 3.9899-0.96664 2.6283-2.8144 3.3937 2.8324 2.3427 4.3898 0.40964 1.3631-2.1816 4.4239 1.2187-1.9616 0.39018-3.6208 2.8189-1.8989 2.0584-3.8989 2.0584 1.7203 3.2483 3.8989 2.0584 2.0942-1.4939 3.6208 2.8189-1.9616-0.39018-4.4239 1.2187-2.542 1.175-4.3898 0.40964 0.34627 3.6593 2.8144 3.3937 2.5065-0.57878 2.2664 3.9899-1.6629-1.1111-4.5535-0.567-2.7982 0.11277-4.2124-1.3014-1.0805 3.5133 1.3014 4.2124 2.5372 0.42446 0.567 4.5535-1.1111-1.6629-3.9899-2.2664-2.6283-0.96664-3.3937-2.8144-2.3427 2.8324-0.40964 4.3898 2.1816 1.3631-1.2187 4.4239-0.39018-1.9616-2.8189-3.6208-2.0584-1.8989-2.0584-3.8989-3.2483 1.7203-2.0584 3.8989 1.4939 2.0942-2.8189 3.6208 0.39018-1.9616-1.2187-4.4239-1.175-2.542-0.40964-4.3898-3.6593 0.34627-3.3937 2.8144 0.57878 2.5065-3.9899 2.2664 1.1111-1.6629 0.567-4.5535-0.11277-2.7982 1.3014-4.2124-3.5133-1.0805-4.2124 1.3014-0.42446 2.5372-4.5535 0.567 1.6629-1.1111 2.2664-3.9899 0.96664-2.6283 2.8144-3.3937-2.8324-2.3427-4.3898-0.40964-1.3631 2.1816-4.4239-1.2187 1.9616-0.39018 3.6208-2.8189 1.8989-2.0584 3.8989-2.0584-1.7203-3.2483-3.8989-2.0584-2.0942 1.4939-3.6208-2.8189 1.9616 0.39018 4.4239-1.2187 2.542-1.175 4.3898-0.40964-0.34627-3.6593-2.8144-3.3937-2.5065 0.57878-2.2664-3.9899 1.6629 1.1111 4.5535 0.567 2.7982-0.11277 4.2124 1.3014 1.0804-3.5133-1.3014-4.2124-2.5372-0.42446-0.567-4.5535 1.1111 1.6629 3.9899 2.2664 2.6283 0.96664 3.3937 2.8144 2.3427-2.8324 0.40964-4.3898-2.1816-1.3631 1.2187-4.4239 0.39018 1.9616 2.8189 3.6208 2.0584 1.8989 2.0584 3.8989m0 2a15 15 0 0 0 0 30 15 15 0 0 0 0-30"/>
                          <path d="m25 22q-3 0-6-3t-9 3q6-3 9 0 2 2 6 0 3 0 6-3t9 3q-6-3-9 0-2 2-6 0"/>
                          <path d="m16.5 19a2.5 2.5 0 0 0 0 5 2.5 2.5 0 0 0 0-5m17 0a2.5 2.5 0 0 0 0 5 2.5 2.5 0 0 0 0-5"/>
                          <path d="m22 25a4 4 0 0 0 6 0 3 4 0 1 1-6 0"/>
                          <path d="m16 29a20 20 0 0 0 18 0 10 10 0 0 1-18 0"/>
                         </g>
                        </svg>)";

    const char sokol[] = "iVBORw0KGgoAAAANSUhEUgAAA5gAAACwCAYAAABq38F7AAAAAXNSR0IArs4c6QAACVxJREFUeJzt3d+"
                         "LnFcZB/CZss0mJiHJNkbFWg2UUKNWQ0GpIJIIJhZ/Ia1Y4520hUIpiEW8C94UaS9aFGmkKogh0kapVs"
                         "SaJilVkEjboFGMunQDJtLuprMxP5psYjL+A5t5hfM9vDOzn8/t5Jz3cN5nntlvzsXp9r/7wU5V0+v7d"
                         "R/AIFM/OTrw896ud3SLHnDDhaLhrXtjRd351f9AI1+fh99Vd/5OR/0M0Fg/37iprH5qO7lmpN9v9f2/"
                         "8UzR8Eb6f6v0/0bqp0WN9fmVDxTVZ/exAyXDh951bS8AAACA8SBgAgAAECFgAgAAECFgAgAAECFgAgA"
                         "AECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAg"
                         "AAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAECFgAgAAENHt793Ub3sRJ"
                         "aYeON32Eor0vrO27SUUadr/3q/f2y16wKrlRcMb/WlG/Q8w9vX54pay+vz7sqLhTabuOVy1Psf+/f70"
                         "Q2Xvt7a542Pdf3pPbC7b/82TRcMb6f8DjX1/0P9rTl9d23//z5+aHe7fl5Y5wQQAACBCwAQAACBCwAQ"
                         "AACBCwAQAACBCwAQAACBCwAQAACBCwAQAACBiou0F1Nbb9b6ye2pevxRaybXMjfQ9XAy33tc2Ft7TNK"
                         "8+W9T7dmH/anTS+4Uxpf+PNv2/TPH+Ha17Btd97EDV+dvmBBMAAIAIARMAAIAIARMAAIAIARMAAIAIA"
                         "RMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAIARMAAIAI"
                         "ARMAAICIbr/3pX7biygxtengwM//8vuPdEvmX7h0pWR4o9u2vTTS+99k/tRs0f6Pe33W1rT//b2bBo6"
                         "feuB01f3v/WNbzemra3q/x45uLar/ybnTJcMbrblx3Vh/v4794fai/d9ww4qx3p/a9P/h3n/9v4z+36"
                         "6m/Z85uKVo/9fe+lzJ8CXPCSYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARE"
                         "20vYKk79swtRff0NJl882LN6Tudt6+rO/+Y+9ezZe9/5fHXGv7F2pLpi9f3fxjpe7hglPWeek/R9/s/"
                         "6/X/Evq//g/jygkmAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQI"
                         "mAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAERNTmw5WfcDvXt7a6vzznU6/6A"
                         "ErikY3enPN6qrzf+y2QwM//8GhO7ol869fdrVkeKOpswtV52/bP992U9H4Lbe/GFrJ4o68+uGq819/9"
                         "nLV+dv27wvXl02w6q2ZhVzDqO9/U/+fK+z/c2fLfj6aNPXn2l559y1F46/r1u3/o16fTfT/8X6/+n+7"
                         "ZirvX6knX7qn7SVU5QQTAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACAiImZv24"
                         "tugexyYmLhfdQAsASdODw9qLf536n6s87ACzKCSYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAi"
                         "YAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARAiYAAAARE"
                         "99/dVXVBzz8mWerzj/qHvnN51qdv7fQ6ZfM/9Wtvxj8/Oc+3S2Zv3N2smj4sHthdnnR+Hv33x9ayeJe"
                         "mL1Udf7VEyuqzt+21y+W/R/eiQsToZVcQ3+09/+hHYP7T6m2+/Mr82X9uWl/7tv3haL+fPPquv1h1Ou"
                         "zif4/3u9X/2/Xy/PLisbX/n5Nn7tadf6bV10p+v1o0vT74gQTAACACAETAACACAETAACACAETAACACA"
                         "ETAACACAETAACACAETAACAiMqX7HQ6j/72jrJ7EBu8c8XlmtN3Ll2tm8FnL5TdcwbA4n54aEfR788bh"
                         "fcEA8BS5AQTAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACACAETAACA"
                         "CAETAACACAETAACACAETAACACAETAACACAETAACACAETAACAiInpc5P9mg+YPldz9k7n6bv3Dfz8o99"
                         "7vlsy/9rJ2ZLhjVa+5edV529bb2GiaPzDn30mtJLF3bX3zlbn/9vZTtH3r6n+P757b1H9dzo/q9ofam"
                         "van9p2bP5xq89vcu/++9teQlWH595fNH73Fx8NrWRxbfef3uWy/vPQ9l8N/Hznnl1F/WfPzl0lwxu1v"
                         "f/6f136/2Dj3v8Pnfhy0fjTCxtCK7mGdU9U/X79cb7dM0QnmAAAAEQImAAAAEQImAAAAEQImAAAAEQI"
                         "mAAAAEQImAAAAEQImAAAAESUXVIIlX3zl58vvMdrsOnzZfeQMdpK6+vkme2ppSzqzH+rln+n03m+8vy"
                         "j7b6nvl71BfSuHNd/BtD/qUn/1/+pxwkmAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQ"
                         "ImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEQImAAAAEd116zf0a"
                         "z5gy+MzNaenwZEHN7b6/PlTs91WF9BA/ber7fq8a++dRfU5PftIainXUrU+R11T/Xziyd1D3X965z85"
                         "0u+39v5PrdxfMrzR03fv0/9bpP83Gun+wHhr+v46wQQAACBCwAQAACBCwAQAACBCwAQAACBCwAQAACB"
                         "CwAQAACBCwAQAACBi4lM/+lbVe8JeO+Men2FW+v5XLf9zaimtOHfxVvU/xJZ6fQL1+PtnuOn/MLqcYA"
                         "IAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABAhY"
                         "AIAABAhYAIAABAhYAIAABAhYAIAABAhYAIAABDR3bbnfNUHHHlwY7/qAyiy5fGZbsn4gztXppbSCvU/"
                         "3OZPzRbVZ23qZ7g11U/t99dk3N+v7+94v9/a1I/6YXQ5wQQAACBCwAQAACBCwAQAACBCwAQAACBCwAQ"
                         "AACBCwAQAACBCwAQAACDifxFf+7S1xG8ZAAAAAElFTkSuQmCC)";

    image_test = frame::image_create(frame::base64_decode(sokol));
    svg_test = frame::svg_parse(sun_svg);
}

void update()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = frame::get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_canvas = frame::get_screen_to_world(mouse_screen);
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

    auto text_center = vec2(100.0f, 100.0f);
    auto text_align = text_align::top_middle;
    auto text_size = 15.0f;
    auto text = "test text first line\nsecond line";
    frame::draw_circle(text_center, 3.0f / frame::get_world_scale().x, col4::RED);
    
    auto rect = frame::get_text_rectangle(text, text_center, text_size, text_align);
    frame::draw_rectangle(rect.center(), rect.size().x, rect.size().y, frame::col4::ORANGE);
    frame::draw_text(text, text_center, text_size, col4::GRAY, text_align);

    frame::free_move_camera_update(free_move_config);

    //frame::draw_svg(svg_test, {10.0f, 10.0f}, frame::text_align::middle_middle);
    //frame::draw_svg_ex(svg_test, { 10.0f, 10.0f }, 45.0f, { 1.0f, 1.0f }, frame::text_align::middle_middle);
    //frame::draw_svg_ex_size(svg_test, { 10.0f, -10.0f }, 0.0f, { 20.0f, 20.0f }, frame::text_align::middle_middle);
    frame::draw_svg_ex_size(svg_test, { 10.0f, 10.0f }, 90.0f, { 80.0f, 80.0f }, frame::text_align::bottom_right);

    auto image_size = frame::get_image_size(image_test);
    frame::draw_circle({ 100.0f, 100.0f }, 3.0f, col4::RED);
    frame::draw_rectangle(frame::rectangle::from_min_max({ 100.0f, 100.0f }, { 300.0f, 50.0f }), col4::BLUE);

    //frame::draw_image(image_test, { 100.0f, 100.0f }, frame::text_align::top_left);
    //frame::draw_image_ex(image_test, { 100.0f, 100.0f }, frame::deg_to_rad(45.0f), {0.2f, 0.2f}, text_align::top_middle);
    //frame::draw_image_ex_size(image_test, { 100.0f, 100.0f }, 0.0f, image_size * 0.3f, text_align::top_left);
    frame::draw_image_ex_size(image_test, { 100.0f, 100.0f }, frame::deg_to_rad(90.0f), {200.0f, 50.0f}, text_align::top_left);

    for (auto& tr : state.rects)
    {
        tr.rotation += 0.5f;
        frame::update_draw_instance(state.rectangle, tr.index, tr.position, tr.rotation, tr.scale, tr.color);
    }

    frame::draw_buffer_instanced(state.rectangle);
    frame::draw_buffer_instanced(state.circle);

    frame::draw_buffer(state.polyline, { 200.0f, 200.0f }, 90.0f, { 0.5f, 0.5f }, frame::col4::WHITE);
    //frame::draw_buffer(state.center, frame::translation({ 0.0f, 0.0f }) * frame::scale({ 200.0f, 200.0f }), frame::col4::WHITE);
    frame::draw_buffer(state.center, frame::translation({ 0.0f, 0.0f }) * frame::scale({ 5.0f, 5.0f }), frame::col4::WHITE);
}
