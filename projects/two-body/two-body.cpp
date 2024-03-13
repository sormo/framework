#include <framework.h>
#include <utils.h>
#include "unit.h"
#include "imgui.h"
#include <string>

using namespace frame;

const double PI = 3.1415926535897931;
double GRAVITATIONAL_CONSTANT = 0.8;
const double DRAW_SIZE_FACTOR = 200.0;
const double DRAW_VELOCITY_FACTOR = 13.0;

vec2 float_cast(const vec2d& p)
{
    return { (float)p.x, (float)p.y };
}

// TODO not needed
vec2d double_cast(const vec2& p)
{
    return { (double)p.x, (double)p.y };
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

free_move_camera_config free_move_config;

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
}

//template <typename T> int sgn(T val)
//{
//    return (T(0) < val) - (val < T(0));
//}
//
//// https://github.com/Karth42/SimpleKeplerOrbits/blob/master/Assets/SimpleKeplerOrbits/Scripts/Utils/KeplerOrbitUtils.cs#L256
//double kepler_solver(double meanAnomaly, double eccentricity)
//{
//    // Iterations count range from 2 to 6 when eccentricity is in range from 0 to 1.
//    int    iterations = (int)(std::ceil((eccentricity + 0.7) * 1.25)) << 1;
//    double m = meanAnomaly;
//    double esinE;
//    double ecosE;
//    double deltaE;
//    double n;
//    for (int i = 0; i < iterations; i++)
//    {
//        esinE = eccentricity * std::sin(m);
//        ecosE = eccentricity * std::cos(m);
//        deltaE = m - esinE - meanAnomaly;
//        n = 1.0 - ecosE;
//        m += -5.0 * deltaE / (n + sgn(n) * std::sqrt(std::abs(16.0 * n * n - 20.0 * deltaE * esinE)));
//    }
//
//    return m;
//}

//vec2d compute_kepler_position(const body& body, double time)
//{
//    // \theta is the true anomaly, which is the angle between the current position of the orbiting object and the location
//    // in the orbit at which it is closest to the central body (called the periapsis).
//
//    double e = body.eccentricity;
//    double a = body.semi_major_axis;
//
//    // mean anomaly, major axis goes to the left
//    double mean_anomaly = ((2.0 * PI) / body.period) * time;
//    if (mean_anomaly > 2.0 * PI)
//        mean_anomaly = std::fmod(mean_anomaly, 2.0 * PI);
//
//    // compute equation M = E - e*sin(E) for E given M (mean anomaly) and e (eccentricity
//    double eccentricAnomaly = kepler_solver(mean_anomaly, e);
//
//    double cosE = std::cos(eccentricAnomaly);
//    double trueAnomaly = std::acos((cosE - e) / (1 - e * cosE));
//
//    if (mean_anomaly > PI)
//        trueAnomaly = 2.0 * PI - trueAnomaly;
//
//    // major axis goes to the left so we will take negative of r
//    double r = -(a * (1 - e * e)) / (1 + e * std::cos(trueAnomaly));
//
//    vec2d rvec = vec2d(r, 0.0);
//    rvec.rotate(trueAnomaly);
//
//    return rvec;
//
//}

// general conic section equation
// Ax^2 + Bxy + Cy^2 + Dx + Ey + F = 0
//struct conic
//{
//    double A = 0.0;
//    double B = 0.0;
//    double C = 0.0;
//    double D = 0.0;
//    double E = 0.0;
//    double F = 0.0;
//
//    double discriminant()
//    {
//        return B*B - 4.0*A*C;
//    }
//
//    bool is_ellipse()
//    {
//        return B*B - 4.0*A*C < 0.0 || 4.0*A*C - B*B > 0.0;
//    }
//
//    // https://math.stackexchange.com/questions/616645/determining-the-major-minor-axes-of-an-ellipse-from-general-form
//    ellipse get_ellipse()
//    {
//        assert(is_ellipse());
//
//        double Q = 64.0*((F*(4.0*A*C - B*B) - A*E*E + B*D*E - C*D*D) / pow((4.0*A*C - B*B), 2.0)); // coefficient normalizing factor 
//        double S = (1.0/4.0)*sqrt(abs(Q)*sqrt(B*B + pow(A-C, 2.0))); // distance between center and focal point
//
//        ellipse result;
//        result.center = { (B*E - 2.0*C*D) / (4.0*A*C - B*B), (B*D - 2.0*A*E) / (4.0*A*C - B*B) };
//        result.semi_major = (1.0/8.0) * sqrt(2.0*abs(Q)*sqrt(B*B + pow(A-C, 2.0)) - 2.0*Q*(A + C));
//        //result.semi_major = (1.0 / 8.0) * sqrt(abs(Q) * sqrt(B * B + pow(A - C, 2.0)) - Q * (A + C));
//        result.semi_minor = sqrt(result.semi_major*result.semi_major - S*S);
//
//        if (Q*A - Q*C == 0.0 && Q*B == 0.0)
//            result.rotation = 0.0;
//        else if (Q*A - Q*C == 0.0 && Q*B > 0.0)
//            result.rotation = PI / 4.0;
//        else if (Q*A - Q*C == 0.0 && Q*B < 0.0)
//            result.rotation = 3.0*PI/4.0;
//        //else if (Q * A - Q * C > 0.0)
//        //    result.rotation = 0.5 * atan2(B, A - C);
//        //else
//        //    result.rotation = 0.5 * atan2(B, A - C) + PI;
//        else if (Q*A - Q*C > 0.0 && Q*B >= 0.0)
//            result.rotation = 0.5*atan2(B, A - C);
//        else if (Q*A - Q*C > 0.0 && Q * B < 0.0)
//            result.rotation = 0.5 * atan2(B, A - C) + PI;
//        else
//            result.rotation = 0.5 * atan2(B, A - C) + 0.5*PI;
//
//        return result;
//    }
//
//    ellipse get_ellipse_completing_square()
//    {
//        assert(B == 0.0);
//
//        // convert from general to standard form:
//        //  (x-h)^2     (y-k)^2
//        // --------- + --------- = 1
//        //    a^2         b^2
//
//        double h = -D / (2.0 * A);
//        double k = -E / (2.0 * C);
//        double a = sqrt((D*D)/(4.0*A*A) + (E*E)/(4.0*A*C) - F*C);
//        double b = sqrt((D*D)/(4.0*A*C) + (E*E)/(4.0*C*C) - F*A);
//
//        ellipse result;
//        result.center = vec2d(h, k);
//        result.semi_major = a;
//        result.semi_minor = b;
//
//        return result;
//    }
//};

double cross(const vec2d& a, const vec2d& b)
{
    return a.x * b.y - b.x * a.y;
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
    double f = 0.0;

    double true_anomaly = acos(e_vec.dot(r2)/(e_vec.length() * r2.length()));
    if (r2.dot(vr2) < 0.0)
        true_anomaly = 2.0 * PI - true_anomaly;

    double o = atan2(e_vec.y, e_vec.x); // argument of periapsis

    return { -e_vec * a, o, a, b, e }; // TODO
}

//conic compute_conic(double centralMass, double orbitingMass, const vec2d& distance, const vec2d& velocity)
//{
//    double G = GRAVITATIONAL_CONSTANT;
//    double reduced_mass = (centralMass * orbitingMass) / (centralMass + orbitingMass);
//    double angular_momentum = reduced_mass * distance.length() * velocity.y; // TODO tangential part of velocity
//    double energy = 0.5 * reduced_mass * pow(velocity.length(), 2) - (G * centralMass * orbitingMass) / distance.length();
//    double semilatus_rectum = pow(angular_momentum, 2) / (reduced_mass * G * centralMass * orbitingMass);
//    double eccentricity = sqrt(1 + (2.0 * energy * pow(angular_momentum, 2)) / (reduced_mass * pow(G * centralMass * orbitingMass, 2)));
//
//    conic ret;
//    ret.A = 1.0 - pow(eccentricity, 2);
//    ret.B = 0.0;
//    ret.C = 1.0;
//    ret.D = -2.0 * eccentricity * semilatus_rectum;
//    ret.E = 0.0;
//    ret.F = -pow(semilatus_rectum, 2);
//
//    return ret;
//}

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

    free_move_config.min_size = { 150.0f, 150.0f };
    free_move_config.boundary = rectangle::from_center_size({ 400.0f, 300.0f }, { 4000.0f, 4000.0f });

    setup_units();
    setup_sun();
    setup_earth();

    //EARTH_ELLIPSE = compute_conic(SUN.mass, EARTH.mass, EARTH.position, EARTH.velocity).get_ellipse();
    //EARTH_ELLIPSE = compute_conic(SUN.mass, EARTH.mass, EARTH.position, EARTH.velocity).get_ellipse_completing_square();
    EARTH_CONIC = compute_conic({}, {}, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);
}

void update()
{
    // draw

    draw_debug_gui();

    draw_coordinate_lines(rgb(50,50,50));

    // sun
    draw_circle({ 0.0f, 0.0f }, scale_independent(5.0f), col4::GOLD);
    // earth
    if (EARTH_CONIC.eccentricity < 1.0)
    {
        draw_ellipse_ex(float_cast(EARTH_CONIC.center * DRAW_SIZE_FACTOR),
                        (float)EARTH_CONIC.rotation,
                        (float)EARTH_CONIC.semi_major * DRAW_SIZE_FACTOR,
                        (float)EARTH_CONIC.semi_minor * DRAW_SIZE_FACTOR,
                        col4::BLANK,
                        scale_independent(1.5f),
                        col4::LIGHTGRAY);
    }
    else
    {
        draw_hyperbola(float_cast(EARTH_CONIC.center * DRAW_SIZE_FACTOR),
                       (float)EARTH_CONIC.rotation,
                       (float)EARTH_CONIC.semi_major * DRAW_SIZE_FACTOR,
                       (float)EARTH_CONIC.semi_minor * DRAW_SIZE_FACTOR,
                       scale_independent(1.5f),
                       col4::LIGHTGRAY);
    }
    draw_circle(float_cast(EARTH_CONIC.center * DRAW_SIZE_FACTOR), 2.5f, rgb(255, 0, 0));
    draw_circle(float_cast(EARTH.position * DRAW_SIZE_FACTOR), scale_independent(5.0f), col4::EARTHBLUE);

    static line_handle_config handle_config;
    handle_config.radius = 10.0f;
    handle_config.outline_color = col4::GREEN;
    handle_config.hover_outline_color = col4::RED;

    vec2 earth_velocity_from = float_cast(EARTH.position * DRAW_SIZE_FACTOR);
    vec2 earth_velocity_to = earth_velocity_from + float_cast(EARTH.velocity * DRAW_VELOCITY_FACTOR);
    draw_line_directed_with_handles(earth_velocity_from, earth_velocity_to, 1.0f, col4::DARKGRAY, &handle_config, &handle_config);

    // update

    auto [new_earth_velocity_from, new_earth_velocity_to] = update_line_with_handles(earth_velocity_from, earth_velocity_to, &handle_config, &handle_config);
    auto earth_velocity_from_delta = new_earth_velocity_from - earth_velocity_from;
    EARTH.position = double_cast(new_earth_velocity_from / DRAW_SIZE_FACTOR);
    EARTH.velocity = double_cast((new_earth_velocity_to - new_earth_velocity_from + earth_velocity_from_delta) / DRAW_VELOCITY_FACTOR);

    EARTH_CONIC = compute_conic({}, {}, SUN.mass, EARTH.position, EARTH.velocity, EARTH.mass);


    free_move_camera_update(free_move_config);
}
