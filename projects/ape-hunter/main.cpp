#include <framework.h>
#include <utils.h>
#include "world.h"
#include "imgui.h"
#include <iostream>
#include <array>
#include "objects.h"
#include "colorLerp.h"
#include "textManager.h"
#include <memory>

//#pragma GCC diagnostic ignored "-Wswitch"

World world;
Objects objects;
ColorLerp colorLerp;
std::unique_ptr<TextManager> textsManager;

struct
{
    Text heading;
    Text velocity;
    Text gravity;
    Text trajectory;
    Text startSimmulation;
}
texts;

enum class State
{
    Intro = 0,
    VelocityVectors,
    GravityVectors,
    Trajectories,
    Stopped,
    Running
}
state;

constexpr std::array<State, 6> StateArray
{
    State::VelocityVectors, // Intro
    State::GravityVectors,  // VelocityVectors
    State::Trajectories,    // GravityVectors
    State::Stopped,         // Trajectories
    State::Running,         // Stopped
    State::Stopped          // Running
};

void startSimulation();
void stopSimulation();

void advanceState()
{
    State newState = StateArray[(size_t)state];

    switch (newState)
    {
    case State::VelocityVectors:
        texts.heading.FadeOut();
        texts.velocity.FadeIn();
        colorLerp.LerpObject(objects.ground, world.GetFill(objects.ground).transparency(255));
        break;
    case State::GravityVectors:
        texts.velocity.FadeOut();
        texts.gravity.FadeIn();
        break;
    case State::Trajectories:
        texts.gravity.FadeOut();
        texts.trajectory.FadeIn();
        break;
    case State::Stopped:
        switch (state)
        {
        case State::Trajectories:
            texts.trajectory.FadeOut();
            texts.startSimmulation.FadeIn();
            break;
        default:
            break;
        }
        stopSimulation();
        break;
    case State::Running:
        if (!texts.startSimmulation.GetHidden())
            texts.startSimmulation.FadeOut();
        startSimulation();
        break;
    default:
        break;
    }

    state = newState;
}

void startSimulation()
{
    world.SetVelocity(objects.projectile, objects.projectileVelocity);
    world.SetGravity({ 0.0f, -GravityAcceleration });
}

void stopSimulation()
{
    destroyObjects(objects);
    objects = createObjects();
}

void createTexts()
{
    texts.heading = textsManager->Text(u8"Kruh a Štvorec")
        .SetPosition(frame::get_world_position_screen_relative({ 0.5f, 0.5f }))
        .SetSize(35.0f)
        .SetColor(frame::col4::RGB(220, 225, 220));

    texts.velocity = textsManager->Text(u8"Kruhu je udelená počiatočná rýchlosť  v ,\nktorej smer ukazuje priamo na štvorec.")
        .SetPosition(frame::get_world_position_screen_relative({ 0.2f, 0.7f }))
        .SetSize(15.0f)
        .SetColor(frame::col4::RGB(220, 225, 220))
        .SetHidden(true);

    texts.gravity = textsManager->Text(u8"Na obe telesá pôsobí gravitačné zrýchlenie  g.")
        .SetPosition(frame::get_world_position_screen_relative({ 0.2f, 0.7f }))
        .SetSize(15.0f)
        .SetColor(frame::col4::RGB(220, 225, 220))
        .SetHidden(true);

    texts.trajectory = textsManager->Text(u8"Štvorec bude padať rovno dolu a kruh\nsa bude pohybovať po parabolickej trajektórii.")
        .SetPosition(frame::get_world_position_screen_relative({ 0.2f, 0.7f }))
        .SetSize(15.0f)
        .SetColor(frame::col4::RGB(220, 225, 220))
        .SetHidden(true);

    texts.startSimmulation = textsManager->Text(u8"Dokázeš nastaviť parametre tak, aby\nkruh trafil padajúci štvorec?\nV ďalšom kroku sa spustí simulácia.")
        .SetPosition(frame::get_world_position_screen_relative({ 0.1f, 0.6f }))
        .SetSize(15.0f)
        .SetColor(frame::col4::RGB(220, 225, 220))
        .SetHidden(true);
}

void setup()
{
    textsManager = std::make_unique<TextManager>(colorLerp);

    ImGui::GetStyle().WindowRounding = 0.0f;
    ImGui::GetStyle().ChildRounding = 0.0f;
    ImGui::GetStyle().FrameRounding = 0.0f;
    ImGui::GetStyle().GrabRounding = 0.0f;
    ImGui::GetStyle().PopupRounding = 0.0f;
    ImGui::GetStyle().ScrollbarRounding = 0.0f;

    frame::set_screen_background(frame::col4::RGBf(0.1f, 0.1f, 0.1f));

    objects = createObjects();
    // set alpha of ground to 0
    world.SetFill(objects.ground, world.GetFill(objects.ground).transparency(0));

    createTexts();

    frame::set_world_transform(frame::translation({ 0.0f, frame::get_screen_size().y }) * frame::scale({ 1.0f, -1.0f }));
}

void drawDebugGui()
{
    ImGui::BeginMainMenuBar();
    
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Mouse");
    auto mouse = frame::get_mouse_screen_position();
    ImGui::Text("%.3f %.3f", mouse.x, mouse.y);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Width");
    ImGui::Text("%.2f", frame::get_screen_size().x);

    ImGui::TextColored(ImVec4(1, 1, 0, 1), "height");
    ImGui::Text("%.2f", frame::get_screen_size().y);

    ImGui::EndMainMenuBar();
}

void drawGui()
{
    bool open = true;
    ImGui::Begin("Settings", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::SetWindowPos({ 0.05f * frame::get_screen_size().x, 0.05f * frame::get_screen_size().x });
    ImGui::SetWindowSize({ 0.3f * frame::get_screen_size().x, 0.0f });

    if (ImGui::SliderFloat("v", &ProjectileVelocityFactor, 50.0f, 200.0f))
        updateProjectileVelocity(objects);
    if (ImGui::SliderFloat("g", &GravityAcceleration, 5.0f, 15.0f))
        updateGravity(objects);
    ImGui::End();
}

// TODO cache those

void drawVelocityVector(Objects& obj)
{
    frame::vec2 projectileVelocity(obj.projectilePosition + (obj.hunterPosition - obj.projectilePosition).normalized() * obj.velocityLength);
    frame::draw_line_directed(obj.projectilePosition, projectileVelocity, frame::col4::RGB(255, 128, 128));
    frame::draw_line_text(obj.projectilePosition, projectileVelocity, "v", 0.9f, 15.0f, 10.0f, frame::side::left, frame::col4::RGB(200, 200, 200));
}

void drawGravityVectors(Objects& obj)
{
    frame::vec2 hunterGravity(obj.hunterPosition.x, obj.hunterPosition.y - obj.gravityLength);
    frame::draw_line_directed(obj.hunterPosition, hunterGravity, frame::col4::RGB(255, 128, 128));
    frame::draw_line_text(obj.hunterPosition, hunterGravity, "g", 0.9f, 15.0f, 10.0f, frame::side::right, frame::col4::RGB(200, 200, 200));

    frame::vec2 projectileGravity(obj.projectilePosition.x, obj.projectilePosition.y - obj.gravityLength);
    frame::draw_line_directed(obj.projectilePosition, projectileGravity, frame::col4::RGB(255, 128, 128));
    frame::draw_line_text(obj.projectilePosition, projectileGravity, "g", 0.9f, 15.0f, 10.0f, frame::side::left, frame::col4::RGB(200, 200, 200));
}

void drawTrajectories(Objects& obj)
{
    static const frame::col4 TrajectoryColor = frame::col4::RGB(50, 50, 50);

    frame::draw_line_solid(obj.hunterPosition, { obj.hunterPosition.x, obj.groundHeight }, TrajectoryColor);

    float v0 = obj.projectileVelocity.length();
    float a = obj.projectileVelocity.angle();
    float g = GravityAcceleration;

    // dlzka vrhu
    float d = (v0 * v0 * std::sinf(2.0f * a)) / g;
    frame::vec2 dp(obj.projectilePosition.x + d, obj.projectilePosition.y);

    frame::draw_circle(dp, 2.0f, frame::col4::RGB(80, 80, 80));

    // vyska vrhu
    float h = (v0 * v0 * std::powf(std::sinf(a), 2.0f)) / (2.0f * g);
    frame::vec2 hp(obj.projectilePosition.x + d / 2.0f, obj.projectilePosition.y + h);

    frame::draw_circle(hp, 2.0f, frame::col4::RGB(80, 80, 80));

    float o = std::tanf(a) * (d / 2.0f);
    frame::vec2 ohp(obj.projectilePosition.x + d / 2.0f, obj.projectilePosition.y + o);

    frame::draw_quad_bezier(obj.projectilePosition, ohp, dp, TrajectoryColor);
}

void update()
{
    world.Draw(Background);

    colorLerp.Update();

    if (frame::is_mouse_released(frame::mouse_button::left))
    {
        advanceState();
    }

    if (frame::is_screen_resized())
    {
        frame::set_world_transform(frame::translation({ 0.0f, frame::get_screen_size().y }) * frame::scale({ 1.0f, -1.0f }));

        destroyObjects(objects);
        objects = createObjects();
    }

    // nvgTextLetterSpacing(vg, 1.5f);
    nvgTextLineHeight(vg, 1.5f);
    textsManager->Draw();

    if (state >= State::VelocityVectors && state != State::Running)
    {
        frame::draw_line_dashed(objects.projectilePosition, objects.hunterPosition, frame::col4::RGB(50, 50, 50));
        drawVelocityVector(objects);
    }

    if (state >= State::GravityVectors && state != State::Running)
        drawGravityVectors(objects);

    if (state >= State::Trajectories && state != State::Running)
        drawTrajectories(objects);

    if (state == State::Running)
        world.Update();

    world.Draw();

    if (state == State::Stopped)
        drawGui();

    drawDebugGui();
}
