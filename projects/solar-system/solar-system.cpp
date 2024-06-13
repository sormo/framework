#include <framework.h>
#include "imgui.h"
#include <fstream>
#include "commons.h"
#include "camera.h"
#include "body_system.h"
#include "view.h"

using namespace frame;

body_system b_system;
camera_type camera;
click_handler left_mouse_click = click_handler(frame::mouse_button::left);

commons::settings_data settings;

bool first_time_init = true;
frame::image sokol_image = 0;

body_node* clicked_body = nullptr;
bool body_view = false;

void setup()
{
    frame::load_font("roboto-medium", "fonts/roboto-medium.ttf");
    frame::load_font("roboto-black", "fonts/roboto-black.ttf");
    frame::load_font("roboto-bold", "fonts/roboto-bold.ttf");
    frame::load_font("roboto", "fonts/roboto.ttf");

    frame::fetch_file("images/sokol_logo.png", [](std::vector<char> data) { sokol_image = frame::image_create(data); });

    //set_world_transform(translation(get_screen_size() / 2.0f) * scale({ 1.0f, -1.0f }));
    set_world_translation(get_screen_size() / 2.0f);
    set_world_scale({ 1.0f, -1.0f });

    camera.setup();

    b_system.setup();
}

void draw_debug_gui()
{
    ImGui::BeginMainMenuBar();

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen");
    auto mouse_screen = get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse_screen.x, mouse_screen.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
    auto mouse_canvas = get_screen_to_world(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", get_screen_size().x, get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", get_world_size().x, get_world_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Translation");
    ImGui::Text("%.2f %.2f ", get_world_translation().x, get_world_translation().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Scale");
    ImGui::Text("%.2f %.2f ", get_world_scale().x, get_world_scale().y);

    ImGui::EndMainMenuBar();

    //ImGui::ShowDemoWindow();
}

void draw_settings_gui()
{
    static bool is_opened = false;
    static const float window_width = 353.0f;

    ImGui::GetStyle().WindowRounding = 7.0f;

    ImGui::SetNextWindowSize({ window_width, 0.0f});
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_::ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10.0f - window_width, 30.0f), ImGuiCond_::ImGuiCond_Always);
    ImGui::Begin("Settings", &is_opened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    
    ImGui::Checkbox("Draw trajectories", &settings.draw_trajectories);
    ImGui::Checkbox("Draw points", &settings.draw_points);
    ImGui::Checkbox("Draw names", &settings.draw_names);
    ImGui::Checkbox("Step time", &settings.step_time);
    ImGui::SliderFloat("Step speed", &settings.step_speed, 0.001f, 10.0f);
    if (ImGui::Combo("Bodies included", (int*)&settings.bodies_included, "more than 100km\0more than 50km\0more than 10km"))
    {
        b_system.setup_bodies(settings.bodies_included);
    }

    ImGui::End();
}

body_node* get_clicked_body()
{
    static const float click_radius_in_pixels = 15.0f;

    return b_system.query(view::get_screen_to_world(get_mouse_screen_position()), click_radius_in_pixels);
}

void evaluate_body_view(bool init = false)
{
    static const double semi_major_axis_pixels_threshold = 10'000.0;

    if (!clicked_body)
    {
        body_view = false;

        return;
    }

    double semi_major_axis_pixels = view::get_world_to_pixel(commons::convert_AU_to_world_size(clicked_body->get_main_body()->orbit.semi_major_axis));

    if ((init || !body_view) && semi_major_axis_pixels > semi_major_axis_pixels_threshold)
    {
        body_view = true;

        // here we again use the fact that we have position relative to main body, so only scale is needed
        view::set_view([body = clicked_body->get_main_body()]() { return commons::draw_cast(body->get_absolute_position()); }, 1000.0);
        camera.follow([body = clicked_body]() { return commons::draw_cast(body->get_main_body_position() * view::get_scale()); });
    }
    else if ((init || body_view) && semi_major_axis_pixels < semi_major_axis_pixels_threshold)
    {
        body_view = false;

        view::clear_view();
        camera.follow([body = clicked_body]() { return commons::draw_cast(body->get_absolute_position()); });
    }
}

void handle_left_click()
{
    if (!left_mouse_click.is_clicked())
        return;

    if (clicked_body = get_clicked_body())
    {
        b_system.set_info(clicked_body);

        evaluate_body_view(true);
    }
    else
    {
        body_view = false;
        clicked_body = nullptr;

        view::clear_view();

        b_system.set_info(nullptr);
        camera.follow(nullptr);
    }
}

void draw_welcome_screen()
{
    auto center = frame::get_world_position_screen_relative({ 0.5f, 0.5f });

    frame::draw_text_ex("Solar System", center, 60.0f, col4::LIGHTGRAY, "roboto-medium", text_align::middle_middle);

    if (sokol_image)
    {
        auto position = frame::get_world_position_screen_relative({ 0.5f, 0.35f });
        auto sokol_size = frame::get_image_size(sokol_image);
        frame::draw_text_ex("made with", position, 15.0f, col4::LIGHTGRAY, "roboto-medium", text_align::top_middle);
        frame::draw_image_ex_size(sokol_image, position - vec2(0.0f, 35.0f), 0.0f, sokol_size * 0.2f, text_align::top_middle);
    }
}

void draw_system()
{
    draw_debug_gui();
    draw_settings_gui();

    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    b_system.draw(body_view ? clicked_body->get_main_body() : nullptr);

    if (settings.body_system_initializing)
        frame::draw_text_ex("loading ...", frame::get_world_position_screen_relative({ 0.5f, 0.5f }), 20.0f, col4::LIGHTGRAY, "roboto-medium", text_align::middle_middle);
}

void update_system()
{
    b_system.update();

    left_mouse_click.update();

    handle_left_click();

    evaluate_body_view();

    camera.update();
}

void update()
{
    // draw
    if (first_time_init && settings.body_system_initializing)
    {
        draw_welcome_screen();
    }
    else
    {
        first_time_init = false;
        draw_system();
    }

    // update
    if (!settings.body_system_initializing)
        update_system();
}
