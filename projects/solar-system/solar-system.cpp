#include <framework.h>
#include "imgui.h"
#include <fstream>
#include "commons.h"
#include "camera.h"
#include "body_system.h"

using namespace frame;

body_system b_system;
camera_type camera;
click_handler left_mouse_click = click_handler(frame::mouse_button::left);

commons::settings_data settings;

bool first_time_init = true;
frame::image sokol_image = 0;

struct system_view
{
    body_node* main_body;
    double scale_factor = 1.0;

    // set the main body (get child of Solar System barycenter)
    void set_body(body_node* body)
    {
        if (main_body)
        {
            frame::set_world_transform(get_transform_to_world());
        }

        if (body == nullptr)
        {
            main_body = nullptr;
            scale_factor = 1.0;
        }
        else
        {
            main_body = body->get_main_body();
            scale_factor = 1000.0;
            frame::set_world_transform(get_transform_to_body());
        }
    }

    // take body transform and create world transform
    frame::mat3 get_transform_to_world()
    {
        auto body_transform = get_world_transform(); // TODO

        auto translation = body_transform.get_translation() - commons::draw_cast(main_body->get_absolute_position()) * body_transform.get_scale() * scale_factor;
        auto scale = body_transform.get_scale() * scale_factor;

        return frame::translation(translation) * frame::scale(scale);
    }

    // take world tansform and create body transform
    frame::mat3 get_transform_to_body()
    {
        //frame::mat3 screen_center = frame::translation(frame::get_screen_size() / 2.0f) * frame::scale(frame::get_world_scale());
        //return screen_center * frame::scale({ 1.0 / scale_factor, 1.0 / scale_factor });

        auto translation = frame::get_screen_size() / 2.0f;
        auto scale = frame::get_world_scale();

        auto world_translation = frame::get_screen_to_world(frame::get_screen_size() / 2.0f) - commons::draw_cast(main_body->get_absolute_position());

        frame::save_world_transform();
        frame::set_world_transform(frame::translation(translation) * frame::scale(scale));

        // TODO cleanup
        frame::set_world_translation(frame::get_screen_size() / 2.0f, world_translation);

        auto result = frame::get_world_transform() * frame::scale({ 1.0 / scale_factor, 1.0 / scale_factor });;

        frame::restore_world_transform();

        return result;
    }
};

system_view view;

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

    return b_system.query(frame::get_mouse_world_position() * view.scale_factor, click_radius_in_pixels);
}

void handle_left_click()
{
    if (!left_mouse_click.is_clicked())
        return;

    if (auto clicked_body = get_clicked_body())
    {
        b_system.set_info(clicked_body);
        view.set_body(clicked_body);
        //camera.follow([clicked_body]() { return commons::draw_cast(clicked_body->get_absolute_position()); });
        camera.follow([clicked_body]() { return commons::draw_cast(clicked_body->get_main_body_position() * view.scale_factor); });
    }
    else
    {
        view.set_body(nullptr);
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

    b_system.draw(view.main_body, view.scale_factor);

    if (settings.body_system_initializing)
        frame::draw_text_ex("loading ...", frame::get_world_position_screen_relative({ 0.5f, 0.5f }), 20.0f, col4::LIGHTGRAY, "roboto-medium", text_align::middle_middle);
}

void update_system()
{
    b_system.update();

    left_mouse_click.update();

    handle_left_click();

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
