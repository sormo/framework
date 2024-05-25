#include <framework.h>
#include "imgui.h"
#include <fstream>
#include "commons.h"
#include "camera.h"
#include "body_system.h"

using namespace frame;

static double time_current = 0.0; // in days
static double time_delta = 1.0 / 1000.0;
//static double time_delta = 0.0;

body_system b_system;
camera_type camera;
click_handler left_mouse_click = click_handler(frame::mouse_button::left);

commons::settings_data settings;

void draw_distance_legend()
{
    auto screen = frame::get_screen_size();

    const float line_size = screen.x / 8.0f;
    const float line_offset = screen.x / 20.0f;
    const float vertical_line_offset = commons::pixel_to_world(5.0f);
    const float line_thickness = commons::pixel_to_world(1.5f);

    vec2 left_point_screen = { screen.x - line_offset - line_size, screen.y - line_offset };
    vec2 right_point_screen = { screen.x - line_offset, screen.y - line_offset };

    vec2 left_point = frame::get_screen_to_world(left_point_screen);
    vec2 right_point = frame::get_screen_to_world(right_point_screen);

    frame::draw_line_solid_ex(left_point, right_point, line_thickness, frame::col4::WHITE);

    auto draw_vertical_line = [vertical_line_offset, line_thickness](const vec2& center)
    {
        frame::draw_line_solid_ex(frame::vec2(center.x, center.y - vertical_line_offset),
                                  frame::vec2(center.x, center.y + vertical_line_offset),
                                  line_thickness,
                                  frame::col4::WHITE);
    };

    draw_vertical_line(left_point);
    draw_vertical_line(right_point);

    vec2 center_point = left_point + (right_point - left_point) / 2.0f;

    double legend_size = (right_point - left_point).length();
    double value_au = commons::convert_world_size_to_AU(legend_size);
    double value_km = commons::convert_AU_to_km(value_au);

    auto text_au = commons::convert_double_to_string(value_au);
    auto text_km = commons::convert_double_to_string(value_km);

    frame::draw_text_ex((text_au + "AU").c_str(), center_point, 15.0f, col4::LIGHTGRAY, "roboto", text_align::bottom_middle);
    frame::draw_text_ex((text_km + "km").c_str(), center_point - vec2(0.0f, commons::pixel_to_world(2.5f)), 15.0f, col4::LIGHTGRAY, "roboto", text_align::top_middle);
}

void draw_current_time()
{
    static const float offset = 20.0f;

    time_t t = 1514764800; // 2018.01.01 00:00:00
    t += (time_current * 86400.0); // convert days to seconds

    auto tm = *std::gmtime(&t);

    char time_str[256] = {};
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S UTC", &tm);

    frame::save_world_transform();
    frame::set_world_transform(frame::identity());

    frame::draw_text_ex(time_str, { offset, get_screen_size().y - offset}, 15.0f, col4::LIGHTGRAY, "roboto", frame::text_align::bottom_left);

    frame::restore_world_transform();
}

void setup_units()
{
    unit::set_base_meter((1.0 / 1.5e8) * 1e-3);
    unit::set_base_kilogram(1.0 / 2e30);
    unit::set_base_second(1e-7 / PI);

    unit::GRAVITATIONAL_CONSTANT = 4.0 * PI * PI;
    //GRAVITATIONAL_CONSTANT = 6.6743e-11 * (unit::meter * unit::meter * unit::meter) / (unit::kilogram * unit::second * unit::second);
}

void setup()
{
    frame::load_font("roboto-medium", "fonts/roboto-medium.ttf");
    frame::load_font("roboto-black", "fonts/roboto-black.ttf");
    frame::load_font("roboto-bold", "fonts/roboto-bold.ttf");
    frame::load_font("roboto", "fonts/roboto.ttf");

    auto size = get_screen_size();

    set_world_transform(translation(size / 2.0f) * scale({ 1.0f, -1.0f }));

    camera.setup();

    setup_units();

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
    auto mouse_canvas = get_world_transform().inverted().transform_point(mouse_screen);
    ImGui::Text("%.3f %.3f", mouse_canvas.x, mouse_canvas.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Screen Size");
    ImGui::Text("%.2f %.2f", get_screen_size().x, get_screen_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Size");
    ImGui::Text("%.2f %.2f ", get_world_size().x, get_world_size().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Translation");
    ImGui::Text("%.2f %.2f ", get_world_translation().x, get_world_translation().y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "World Scale");
    ImGui::Text("%.2f %.2f ", get_world_scale().x, get_world_scale().y);

    //ImGui::TextColored(ImVec4(1, 1, 0, 1), "Jupiter Point Count");
    //ImGui::Text("%d", ellipse_point_count(5.0, frame::get_world_scale().x));

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
    static float step_speed = (float)time_delta;
    if (ImGui::SliderFloat("Step speed", &step_speed, 0.001f, 10.0f))
        time_delta = step_speed;
    if (ImGui::Combo("Bodies included", (int*)&settings.bodies_included, "more than 100km\0more than 50km\0more than 10km"))
    {
        b_system.setup_bodies(settings.bodies_included);
    }

    ImGui::End();
}

body_node* get_clicked_body()
{
    static const float click_radius_in_pixels = 15.0f;

    return b_system.query(frame::get_mouse_world_position(), click_radius_in_pixels);
}

void handle_left_click()
{
    if (!left_mouse_click.is_clicked())
        return;

    if (auto clicked_body = get_clicked_body())
    {
        b_system.set_info(clicked_body);
        camera.follow([clicked_body]() { return commons::draw_cast(clicked_body->get_absolute_position()); });
    }
    else
    {
        b_system.set_info(nullptr);
        camera.follow(nullptr);
    }
}

void update()
{
    // draw

    draw_debug_gui();
    draw_settings_gui();

    draw_coordinate_lines(rgb(40, 40, 40));

    frame::nanovg_flush();

    b_system.draw();

    draw_distance_legend();
    draw_current_time();

    // update

    b_system.update(time_delta);

    left_mouse_click.update();

    handle_left_click();

    camera.update();

    if (settings.step_time)
        time_current += time_delta;
}
