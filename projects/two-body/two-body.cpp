#include <framework.h>
#include <utils.h>
#include "unit.h"
#include "imgui.h"
#include "kepler_orbit.h"
#include <string>
#include <fstream>
#include "json.hpp"

using namespace frame;

double GRAVITATIONAL_CONSTANT = 0.8;
const double DRAW_SIZE_FACTOR = 200.0;
const double DRAW_VELOCITY_FACTOR = 13.0;

static double time_current = 0.0f;
static double time_delta = 1.0 / 1000.0;

vec2 float_cast(const vec2d& p)
{
    return { (float)p.x, (float)p.y };
}

float scale_independent(float s)
{
    return s / get_world_scale().x;
}

struct body
{
    std::string name;
    vec2d position;
    vec2d velocity;
    double mass = 0.0;
    double radius = 0.0;
};

struct conic
{
    vec2d center;
    double rotation = 0.0;
    double semi_major = 0.0;
    double semi_minor = 0.0;
    double eccentricity = 0.0;
};

body SUN;
body EARTH;
conic EARTH_CONIC;
conic SUN_CONIC;

free_move_camera_config free_move_config;

struct planet_data
{
    void init(const vec2d& r1_o, double m1_o, const vec2d& r2_o, const vec2d& v2_o, double m2_o)
    {
        r2 = r2_o;
        p2 = v2_o * m2_o;
        m2 = m2_o;

        r1 = r1_o;
        m1 = m1_o;
        p1 = -p2;
    }

    vec2d r1;
    vec2d p1;
    double m1;

    vec2d r2;
    vec2d p2;
    double m2;

} planet_data_sim;


struct kepler_orbit_two_bodies
{
    void initialize(const vec2d& r1, const vec2d& v1, double m1, const vec2d& r2, const vec2d& v2, double m2)
    {
        barycenter = (r1 * m1 + r2 * m2) / (m1 + m2);

        vec2d br1 = r1 - barycenter;
        vec2d br2 = r2 - barycenter;
        //double bm1 = (m2 * pow(br1.length(), 2.0)) / pow((r1 - r2).length(), 2.0);
        //double bm2 = (m1 * pow(br2.length(), 2.0)) / pow((r1 - r2).length(), 2.0);

        auto reduced_mass = (m1 * m2) / (m1 + m2);
        auto barycenter_velocity = (v1 * m1 + v2 * m2) / (m1 + m2);

        double bm1 = m2 + m1;
        double bm2 = m1 + m2;

        double r = (r2 - r1).length();
        double v = v2.length();

        double new_r = (r2 - barycenter).length();
        double new_v = (r * v) / new_r;
        //vec2d new_vec = v2.normalized() * new_v;

        //vec2d new_vec1(0.0, sqrt((GRAVITATIONAL_CONSTANT * bm1) / br1.length()));
        //vec2d new_vec2(0.0, sqrt((GRAVITATIONAL_CONSTANT * bm2) / br2.length()));
        vec2d new_vec2 = v2 + v1;
        vec2d new_vec1 = new_vec2 * (m1 / m2);

        orbit1.initialize(br1, new_vec1, bm1, reduced_mass, GRAVITATIONAL_CONSTANT);
        orbit2.initialize(br2, new_vec2, bm2, reduced_mass, GRAVITATIONAL_CONSTANT);

        kepler_orbit reduced_orbit;
        reduced_orbit.initialize(r2 - r1, v1 + v2, m1 + m2, 0.0, GRAVITATIONAL_CONSTANT);
        reduced_orbit.center_point -= vec3d(barycenter);

        orbit2 = reduced_orbit;
    }

    vec2d barycenter;

    kepler_orbit orbit1;
    kepler_orbit orbit2;
};

kepler_orbit_two_bodies kepler;

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

    ImGui::EndMainMenuBar();

    //ImGui::ShowDemoWindow();
    static bool is_open = true;
    ImGui::Begin("Dear ImGui Demo", &is_open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    ImGui::PushItemWidth(80.0f);
    static char earth_weight[128] = "5.974e24";
    if (ImGui::InputText("Earth weight [kg]", earth_weight, 128, ImGuiInputTextFlags_CharsScientific))
    {
        double weight = std::stod(earth_weight);
        EARTH.mass = weight * unit::kilogram;

        planet_data_sim.init(SUN.position, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);
        kepler.initialize(SUN.position, SUN.velocity, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);
    }
    static char sun_weight[128] = "1.989e30";
    if (ImGui::InputText("Sun weight [kg]", sun_weight, 128, ImGuiInputTextFlags_CharsScientific))
    {
        double weight = std::stod(sun_weight);
        SUN.mass = weight * unit::kilogram;

        planet_data_sim.init(SUN.position, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);
        kepler.initialize(SUN.position, SUN.velocity, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);
    }
    ImGui::PopItemWidth();

    ImGui::End();
}

double cross(const vec2d& a, const vec2d& b)
{
    return a.x * b.y - b.x * a.y;
}

conic compute_conic(const vec2d& r1, double m1, const vec2d& r2, const vec2d& v2, double m2)
{
    vec2d r = r2 - r1;
    double G = GRAVITATIONAL_CONSTANT;
    double mu = G * (m1 + m2);

    vec2d e_vec = r * (v2.length_sqr() / mu - 1 / r.length()) - v2 * (r.dot(v2) / mu); // eccentricity vector
    double e = e_vec.length();

    double h = cross(r, v2); // specific angular momentum
    double E = v2.length_sqr() / 2.0 - mu / r.length(); // specific orbital energy
    double a = -mu / (2.0 * E); // semi-major axis
    double b = e < 1.0 ? a * sqrt(1.0 - e * e) : a * sqrt(e * e - 1.0);

    double true_anomaly = acos(e_vec.dot(r) / (e_vec.length() * r.length()));
    if (r.dot(v2) < 0.0)
        true_anomaly = 2.0 * PI - true_anomaly;

    double o = atan2(e_vec.y, e_vec.x); // argument of periapsis

    return { -e_vec * a, o, a, b, e }; // TODO
}

conic compute_conic(const vec2d& p1, const vec2d& v1, double m1, const vec2d& p2, const vec2d& v2, double m2)
{
    vec2d r2 = p2 - p1; // vector from p1 to p2
    vec2d vr2 = v2 - v1; // relative motion of p2 ?

    double G = GRAVITATIONAL_CONSTANT;
    double M = m1 + m2; // total mass

    double mu = G * (m1 + m2);
    //double a = -mu / (vr2.length_sqr() - (2.0*mu)/(r2.length()));

    vec2d e_vec = r2 * (vr2.length_sqr() / mu - 1 / r2.length()) - vr2 * (r2.dot(vr2) / mu); // eccentricity vector
    double e = e_vec.length();

    double h = cross(r2, vr2); // specific angular momentum
    //double L = h / M; // specific angular momentum
    double E = vr2.length_sqr() / 2.0 - mu / r2.length(); // specific orbital energy
    double a = -mu / (2.0 * E); // semi-major axis
    //double a = mu / ((2.0 * mu) / r2.length() - vr2.length_sqr()); // vis-viva
    //double E = -mu / (2.0 * a); // specific orbital energy
    //double e = sqrt(1.0 - (h * h) / (mu * a));
    double b = e < 1.0 ? a * sqrt(1.0 - e * e) : a * sqrt(e * e - 1.0);

    double true_anomaly = acos(e_vec.dot(r2)/(e_vec.length() * r2.length()));
    if (r2.dot(vr2) < 0.0)
        true_anomaly = 2.0 * PI - true_anomaly;

    double o = atan2(e_vec.y, e_vec.x); // argument of periapsis

    return { -e_vec * a, o, a, b, e }; // TODO
}

std::pair<conic, conic> compute_two_conics(const vec2d& r1, const vec2d& v1, double m1, const vec2d& r2, const vec2d& v2, double m2)
{
    auto barycenter = (r1 * m1 + r2 * m2) / (m1 + m2);
    draw_circle(float_cast(barycenter * DRAW_SIZE_FACTOR), 2.0f, col4::BLUE);

    auto reduced_mass = (m1 * m2) / (m1 + m2);
    auto barycenter_velocity = (v1 * m1 + v2 * m2) / (m1 + m2);

    auto c1 = compute_conic(barycenter, barycenter_velocity, m1 + m2, r2 - barycenter, v2, m2);
    auto c2 = compute_conic(barycenter, barycenter_velocity, m1 + m2, r1 - barycenter, v1, m1);

    return { c1, c2 };
}

void draw_conic(const conic& con, const col4& color)
{
    if (con.eccentricity < 1.0)
    {
        draw_ellipse_ex(float_cast(con.center * DRAW_SIZE_FACTOR),
            (float)con.rotation,
            (float)con.semi_major * DRAW_SIZE_FACTOR,
            (float)con.semi_minor * DRAW_SIZE_FACTOR,
            col4::BLANK,
            scale_independent(1.5f),
            color);
    }
    else
    {
        draw_hyperbola(float_cast(con.center * DRAW_SIZE_FACTOR),
            (float)con.rotation,
            (float)con.semi_major * DRAW_SIZE_FACTOR,
            (float)con.semi_minor * DRAW_SIZE_FACTOR,
            scale_independent(1.5f),
            color);
    }
    draw_circle(float_cast(con.center * DRAW_SIZE_FACTOR), scale_independent(2.5f), rgb(255, 0, 0));
}

std::vector<vec2> float_cast(const std::vector<vec2d>& data)
{
    std::vector<vec2> r;
    r.reserve(data.size());
    for (const auto& o : data)
        r.push_back(float_cast(o) * DRAW_SIZE_FACTOR);
    return r;
}

std::vector<vec2> float_cast(const std::vector<vec3d>& data)
{
    std::vector<vec2> r;
    r.reserve(data.size());
    for (const auto& o : data)
        r.push_back(float_cast(o.xy<double>()) * DRAW_SIZE_FACTOR);
    return r;
}

std::vector<vec2>& offset(std::vector<vec2>& data, const vec2& offset)
{
    for (auto& d : data)
        d += offset;
    return data;
}

// https://kyleniemeyer.github.io/space-systems-notes/orbital-mechanics/two-body-problems.html
std::vector<vec2d> compute_orbit_test(double mu, const vec2d& r2, const vec2d& v2)
{
    std::vector<vec2d> result;

    double r0_mag = r2.length();
    double v0_mag = v2.length();
    double h_mag = r2.cross(v2);
    double v_r0 = (r2.dot(v2)) / r2.length();
    //double mu = GRAVITATIONAL_CONSTANT * (m1 + m2);

    double theta_0 = atan(r2.y / r2.x);
    double e = ((pow(h_mag, 2.0) / (mu * r0_mag)) - 1) / cos(theta_0);

    for (double delta_theta = 0.0f; delta_theta < 2.0 * PI; delta_theta += 0.1)
    {
        //3. Get new distance
        double r_mag = (pow(h_mag, 2.0) / mu) / (1 + ((pow(h_mag, 2.0) / (mu * r0_mag)) - 1) * cos(delta_theta) - h_mag * v_r0 * sin(delta_theta) / mu);

        //4. Get Lagrange coefficients
        double f = 1 - mu * r_mag * (1 - cos(delta_theta)) / pow(h_mag, 2.0);
        double g = r_mag * r0_mag * sin(delta_theta) / h_mag;
        double fdot = (mu / h_mag) * (1 - cos(delta_theta)) * (mu * (1 - cos(delta_theta)) / pow(h_mag, 2.0) - (1 / r0_mag) - (1 / r_mag)) / sin(delta_theta);
        double gdot = 1 - mu * r0_mag * (1 - cos(delta_theta)) / pow(h_mag, 2.0);

        // 5. Calculate new vectors:
        vec2d r = r2 * f + v2 * g;
        vec2d v = r2 * fdot + v2 * gdot;

        result.push_back(r);
    }

    return result;
}

conic compute_conic_test(double mu, const vec2d& r2, const vec2d& v2)
{
    double r0_mag = r2.length();
    double v0_mag = v2.length();
    double h_mag = r2.cross(v2);
    double v_r0 = (r2.dot(v2)) / r2.length();
    double theta_0 = atan(r2.y / r2.x);
    //double e2 = ((pow(h_mag, 2.0) / (mu * r0_mag)) - 1) / cos(theta_0);

    vec2d e_vec = r2 * (v2.length_sqr() / mu - 1 / r2.length()) - v2 * (r2.dot(v2) / mu); // eccentricity vector
    double e = e_vec.length();

    double E = v2.length_sqr() / 2.0 - mu / r2.length(); // specific orbital energy
    double a = -mu / (2.0 * E); // semi-major axis
    double b = e < 1.0 ? a * sqrt(1.0 - e * e) : a * sqrt(e * e - 1.0);
    double o = atan2(e_vec.y, e_vec.x); // argument of periapsis

    return { -e_vec * a, o, a, b, e }; // TODO
}

void draw_two_orbits_test(const vec2d& r1, const vec2d& v1, double m1, const vec2d& r2, const vec2d& v2, double m2)
{
    auto barycenter = (r1 * m1 + r2 * m2) / (m1 + m2);
    draw_circle(float_cast(barycenter * DRAW_SIZE_FACTOR), 2.0f, col4::BLUE);

    auto reduced_mass = (m1 * m2) / (m1 + m2);
    auto barycenter_velocity = (v1 * m1 + v2 * m2) / (m1 + m2);
    
    // --

    double mu = GRAVITATIONAL_CONSTANT * (m1 + m2);
    auto r1b = r1 - barycenter;
    auto v1b = v1 - v2;
    auto r2b = r2 - barycenter;
    auto v2b = v2 - v1;

    // --

    auto earth_test = float_cast(compute_orbit_test(mu, r2b, v2b));
    draw_polyline(offset(earth_test, float_cast(barycenter * DRAW_SIZE_FACTOR)), col4::GREEN);

    auto earth_conic = compute_conic_test(mu, r2b, v2b);
    earth_conic.center += barycenter;
    draw_conic(earth_conic, col4::GRAY);

    auto sun_test = float_cast(compute_orbit_test(mu, r1b, v1b));
    draw_polyline(offset(sun_test, float_cast(barycenter * DRAW_SIZE_FACTOR)), col4::RED);

    auto sun_conic = compute_conic_test(mu, r1b, v1b);
    sun_conic.center += barycenter;
    draw_conic(sun_conic, col4::GRAY);
}

void simulate_two_orbits_test(planet_data& data)
{
    vec2d r = data.r2 - data.r1;
    vec2d f2 = -r.normalized() * GRAVITATIONAL_CONSTANT * data.m1 * data.m2 / pow(r.length(), 2.0);

    data.p2 = data.p2 + f2 * time_delta; // force applied over time is impulse - change in momentum
    data.p1 = data.p1 - f2 * time_delta;
    data.r1 = data.r1 + data.p1 * time_delta / data.m1;
    data.r2 = data.r2 + data.p2 * time_delta / data.m2;

    draw_circle(float_cast(data.r1) * DRAW_SIZE_FACTOR, 3.0, col4::RED);
    draw_circle(float_cast(data.r2) * DRAW_SIZE_FACTOR, 3.0, col4::RED);
}

void kepler_test()
{
    kepler.orbit1.update_current_orbit_time_by_delta_time(time_delta);
    kepler.orbit2.update_current_orbit_time_by_delta_time(time_delta);
    //orbit_sim.set_current_orbit_time(time);

    vec2 barycenter = float_cast(kepler.barycenter * DRAW_SIZE_FACTOR);
    draw_circle(barycenter, 5.0f, col4::BLUE);

    draw_circle(float_cast(kepler.orbit1.position.xy<double>() * DRAW_SIZE_FACTOR) + barycenter, 5.0f, col4::GOLD);
    draw_circle(float_cast(kepler.orbit2.position.xy<double>() * DRAW_SIZE_FACTOR) + barycenter, 5.0f, col4::EARTHBLUE);

    auto points1 = kepler.orbit1.get_orbit_points();
    draw_polyline(offset(float_cast(points1), barycenter), col4::GREEN);

    auto points2 = kepler.orbit2.get_orbit_points();
    draw_polyline(offset(float_cast(points2), barycenter), col4::GREEN);
}

void setup_units()
{
    unit::set_base_meter((1.0 / 1.5e8) * 1e-3);
    unit::set_base_kilogram(1.0 / 2e30);
    unit::set_base_second(1e-7 / PI);

    GRAVITATIONAL_CONSTANT = 4.0 * PI * PI;
    //GRAVITATIONAL_CONSTANT = 6.6743e-11 * (unit::meter * unit::meter * unit::meter) / (unit::kilogram * unit::second * unit::second);
}

void setup_sun()
{
    SUN.name = "sun";
    SUN.position = { 0.0 * unit::meter, 0.0 * unit::meter };
    SUN.velocity = { 0.0 * unit::meter / unit::second, 0.0 * unit::meter / unit::second };
    SUN.mass = 1.989e30 * unit::kilogram;
    SUN.radius = 6.957e8 * unit::meter;
}

void setup_earth()
{
    EARTH.name = "earth";
    //EARTH.position = { 1.496e11 * unit::meter, 0.0 * unit::meter };
    EARTH.position = { 1.4e11 * unit::meter, 0.0 * unit::meter };
    //EARTH.velocity = { 0.0 * unit::meter / unit::second, 2.98e4 * unit::meter / unit::second };
    EARTH.velocity = { 0.0 * unit::meter / unit::second, 1.5e4 * unit::meter / unit::second };
    EARTH.mass = 5.974e24 * unit::kilogram;
    EARTH.radius = 6.371e6 * unit::meter;
}

void setup()
{
    auto size = get_screen_size();

    set_world_transform(translation(size / 2.0f) * scale({ 1.0f, -1.0f }));

    free_move_config.min_size = { 1.0f, 1.0f };
    free_move_config.boundary = rectangle::from_center_size({ 400.0f, 300.0f }, { 100000.0f, 100000.0f });

    setup_units();
    setup_sun();
    setup_earth();

    std::tie(EARTH_CONIC, SUN_CONIC) = compute_two_conics({}, {}, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);

    planet_data_sim.init(SUN.position, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);
    kepler.initialize(SUN.position, SUN.velocity, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);
}

void update()
{
    draw_debug_gui();

    draw_coordinate_lines(rgb(50,50,50));

    kepler_test();

    simulate_two_orbits_test(planet_data_sim);

    free_move_camera_update(free_move_config);

    time_current += time_delta;
}
