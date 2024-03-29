#include "kepler_orbit.h"

using namespace frame;

void kepler_orbit::initialize(vec3d pos, vec3d vel, double attr_mass, double t_mass, double g_constant)
{
    position = pos;
    velocity = vel;
    attractor_mass = attr_mass;
    this_mass = t_mass;
    gravitational_constant = g_constant;

    calculate_initial_orbit_state();
}

void kepler_orbit::initialize(double eccentricity, vec3d semi_major_axis, vec3d semi_minor_axis, double mean_anomaly_radians, double attractor_mass, double g_constant)
{
    this->eccentricity = eccentricity;
    this->semi_major_axis_basis = semi_major_axis.normalized();
    this->semi_minor_axis_basis = semi_minor_axis.normalized();
    this->semi_major_axis = semi_major_axis.length();
    this->semi_minor_axis = semi_minor_axis.length();

    this->mean_anomaly = mean_anomaly_radians;
    this->eccentric_anomaly = convert_mean_to_eccentric_anomaly(mean_anomaly, eccentricity);
    this->true_anomaly = convert_eccentric_to_true_anomaly(eccentric_anomaly, eccentricity);
    this->attractor_mass = attractor_mass;
    this->gravitational_constant = g_constant;

    calculate_orbit_state_from_orbital_elements();
}

void kepler_orbit::initialize(double eccentricity,
                              double semi_major_axis,
                              double mean_anomaly_rad,
                              double inclination_rad,
                              double arg_of_perifocus_rad,
                              double ascending_node_rad,
                              double attractor_mass,
                              double g_constant)
{
    this->eccentricity = eccentricity;
    this->semi_major_axis = semi_major_axis;
    if (eccentricity < 1.0)
    {
        this->semi_minor_axis = semi_major_axis * sqrt(1 - eccentricity * eccentricity);
    }
    else if (eccentricity > 1.0)
    {
        this->semi_minor_axis = semi_major_axis * sqrt(eccentricity * eccentricity - 1);
    }
    else
    {
        this->semi_major_axis = 0;
    }


    auto normal = vec3d(0, 0, 1);
    auto ascending_node = vec3d(1, 0, 0);

    ascending_node_rad = fmod(ascending_node_rad, 2.0 * PI);
    if (ascending_node_rad > PI) ascending_node_rad -= 2.0 * PI;
    inclination_rad = fmod(inclination_rad, 2.0 * PI);
    if (inclination_rad > PI) inclination_rad -= 2.0 * PI;
    arg_of_perifocus_rad = fmod(arg_of_perifocus_rad, 2.0 * PI);
    if (arg_of_perifocus_rad > PI) arg_of_perifocus_rad -= 2.0 * PI;

    ascending_node = ascending_node.rotated(ascending_node_rad, normal).normalized();
    normal = normal.rotated(inclination_rad, ascending_node).normalized();
    periapsis = ascending_node;
    periapsis = periapsis.rotated(arg_of_perifocus_rad, normal).normalized();

    this->semi_major_axis_basis = periapsis;
    this->semi_minor_axis_basis = periapsis.cross(normal);

    this->mean_anomaly = mean_anomaly_rad;
    this->eccentric_anomaly = convert_mean_to_eccentric_anomaly(this->mean_anomaly, this->eccentricity);
    this->true_anomaly = convert_eccentric_to_true_anomaly(this->eccentric_anomaly, this->eccentricity);
    this->attractor_mass = attractor_mass;
    this->gravitational_constant = g_constant;

    calculate_orbit_state_from_orbital_elements();
}

kepler_orbit::kepler_orbit()
{}

kepler_orbit::kepler_orbit(vec3d pos, vec3d vel, double attractor_mass, double this_mass, double gravitational_constant)
    : position(pos), velocity(vel), attractor_mass(attractor_mass), this_mass(this_mass), gravitational_constant(gravitational_constant)
{
    calculate_initial_orbit_state();
}

std::vector<vec3d> kepler_orbit::get_orbit_points(int points_count, double max_distance)
{
    return get_orbit_points(points_count, {}, max_distance);
}

std::vector<vec3d> kepler_orbit::get_orbit_points(int points_count, const vec3d& origin, double max_distance)
{
    if (points_count < 2)
    {
        return {};
    }

    std::vector<vec3d> result(points_count);
    if (eccentricity < 1.0)
    {
        if (apoapsis_distance < max_distance)
        {
            for (int i = 0; i < points_count; i++)
            {
                result[i] = get_focal_position_at_eccentric_anomaly(i * (2.0 * PI) / (points_count - 1.0)) + origin;
            }
        }
        else if (eccentricity > 1.0)
        {
            double max_angle = calc_true_anomaly_for_distance(max_distance, eccentricity, semi_major_axis, periapsis_distance);
            for (int i = 0; i < points_count; i++)
            {
                result[i] = get_focal_position_at_true_anomaly(-max_angle + i * 2.0 * max_angle / (points_count - 1.0)) + origin;
            }
        }
        else
        {
            double max_angle = calc_true_anomaly_for_distance(max_distance, eccentricity, periapsis_distance, periapsis_distance);
            for (int i = 0; i < points_count; i++)
            {
                result[i] = get_focal_position_at_true_anomaly(-max_angle + i * 2.0 * max_angle / (points_count - 1.0)) + origin;
            }
        }
    }
    else
    {
        if (max_distance < periapsis_distance)
        {
            return {};
        }

        double max_angle = calc_true_anomaly_for_distance(max_distance, eccentricity, semi_major_axis, periapsis_distance);

        for (int i = 0; i < points_count; i++)
        {
            result[i] = get_focal_position_at_true_anomaly(-max_angle + i * 2.0 * max_angle / (points_count - 1.0)) + origin;
        }
    }

    return result;
}

double kepler_orbit::get_current_orbit_time()
{
    if (eccentricity < 1.0)
    {
        if (period > 0 && period < std::numeric_limits<double>::max())
        {
            auto anomaly = fmod(mean_anomaly, 2.0 * PI);
            if (anomaly < 0.0)
            {
                anomaly = 2.0 * PI - anomaly;
            }

            return anomaly / (2.0 * PI) * period;
        }

        return 0.0;
    }
    else if (eccentricity > 1.0)
    {
        if (mean_motion > 0.0)
        {
            return mean_anomaly / mean_motion;
        }

        return 0.0;
    }
    else
    {
        if (mean_motion > 0)
        {
            return mean_anomaly / mean_motion;
        }

        return 0.0;
    }
}

// TODO test

void kepler_orbit::set_current_orbit_time(double time)
{
    mean_anomaly = mean_anomaly_initial + mean_motion * time;
    process_updated_mean_anomaly();

    set_position_by_current_anomaly();
    set_velocity_by_current_anomaly();
}

void kepler_orbit::update_current_orbit_time_by_delta_time(double delta_time)
{
    mean_anomaly += mean_motion * delta_time;
    process_updated_mean_anomaly();

    set_position_by_current_anomaly();
    set_velocity_by_current_anomaly();
}

double kepler_orbit::calc_true_anomaly_for_distance(double distance, double eccentricity, double semi_major_axis, double periapsis_distance)
{
    if (eccentricity < 1.0)
    {
        return acos((semi_major_axis * (1.0 - eccentricity * eccentricity) - distance) / (distance * eccentricity));
    }
    else if (eccentricity > 1.0)
    {
        return acos((semi_major_axis * (eccentricity * eccentricity - 1.0) - distance) / (distance * eccentricity));
    }
    else
    {
        return acos((periapsis_distance / distance) - 1.0);
    }
}

void kepler_orbit::calculate_orbit_state_from_orbital_elements()
{
    static const vec3d ecliptic_normal(0, 0, 1);

    mu = gravitational_constant * (attractor_mass + this_mass);

    orbit_normal = -semi_major_axis_basis.cross(semi_minor_axis_basis).normalized();
    orbit_normal_dot_ecliptic_normal = orbit_normal.dot(ecliptic_normal);

    if (eccentricity < 1.0)
    {
        orbit_compression_ratio = 1 - eccentricity * eccentricity;
        center_point = -semi_major_axis_basis * semi_major_axis * eccentricity;
        if (mu != 0.0)
        {
            period = (2.0 * PI) * sqrt(pow(semi_major_axis, 3.0) / mu);
            mean_motion = (2.0 * PI) / period;
        }
        apoapsis = center_point - semi_major_axis_basis * semi_major_axis;
        periapsis = center_point + semi_major_axis_basis * semi_major_axis;
        periapsis_distance = periapsis.length();
        apoapsis_distance = apoapsis.length();
        // All anomalies state already preset.
    }
    else if (eccentricity > 1.0)
    {
        center_point = semi_major_axis_basis * semi_major_axis * eccentricity;
        period = std::numeric_limits<double>::max();
        if (semi_major_axis != 0.0)
            mean_motion = sqrt(mu / pow(semi_major_axis, 3.0));
        apoapsis = vec3d(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        periapsis = center_point - semi_major_axis_basis * (semi_major_axis);
        periapsis_distance = periapsis.length();
        apoapsis_distance = std::numeric_limits<double>::max();
    }
    else
    {
        center_point = {};
        period = std::numeric_limits<double>::max();
        if (periapsis_distance != 0.0)
            mean_motion = sqrt(mu * 0.5 / pow(periapsis_distance, 3.0));
        apoapsis = vec3d(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        periapsis_distance = semi_major_axis;
        semi_major_axis = 0;
        periapsis = semi_major_axis_basis * (-periapsis_distance);
        apoapsis_distance = std::numeric_limits<double>::max();
    }

    position = get_focal_position_at_eccentric_anomaly(eccentric_anomaly);
    double compresion = eccentricity < 1 ? (1 - eccentricity * eccentricity) : (eccentricity * eccentricity - 1);
    focal_parameter = semi_major_axis * compresion;
    velocity = get_velocity_at_true_anomaly(true_anomaly);
    attractor_distance = position.length();
}

void kepler_orbit::calculate_initial_orbit_state()
{
    static const vec3d ecliptic_up(0, 1, 0); // up-direction on xy plane
    static const vec3d ecliptic_normal(0, 0, 1);

    mu = gravitational_constant * (attractor_mass + this_mass);
    attractor_distance = position.length();

    vec3d position3(position);
    vec3d velocity3(velocity);

    vec3d angular_momentum_vector = position3.cross(velocity3);
    orbit_normal = angular_momentum_vector.normalized();

    vec3d eccentricity_vector;
    if (orbit_normal.length_sqr() < 0.99)
    {
        orbit_normal = position3.cross(ecliptic_up).normalized();
        eccentricity_vector = {};
    }
    else
    {
        eccentricity_vector = velocity.cross(angular_momentum_vector) / mu - position / attractor_distance;
    }

    orbit_normal_dot_ecliptic_normal = orbit_normal.dot(ecliptic_normal);
    focal_parameter = angular_momentum_vector.length_sqr() / mu;
    eccentricity = eccentricity_vector.length();

    semi_minor_axis_basis = angular_momentum_vector.cross(-eccentricity_vector).normalized();
    if (semi_minor_axis_basis.length_sqr() < 0.99)
    {
        semi_minor_axis_basis = orbit_normal.cross(position).normalized();
    }

    semi_major_axis_basis = orbit_normal.cross(semi_minor_axis_basis).normalized();
    if (eccentricity < 1.0)
    {
        orbit_compression_ratio = 1 - eccentricity * eccentricity;
        semi_major_axis = focal_parameter / orbit_compression_ratio;
        semi_minor_axis = semi_major_axis * sqrt(orbit_compression_ratio);
        center_point = -eccentricity_vector * semi_major_axis;
        double p = sqrt(pow(semi_major_axis, 3.0) / mu);
        period = 2.0 * PI * p;
        mean_motion = 1.0 / p;
        apoapsis = center_point - semi_major_axis_basis * semi_major_axis;
        periapsis = center_point + semi_major_axis_basis * semi_major_axis;
        periapsis_distance = periapsis.length();
        apoapsis_distance = apoapsis.length();
        true_anomaly = position.angle(semi_major_axis_basis);
        if (position.cross(-semi_major_axis_basis).dot(orbit_normal) < 0)
        {
            true_anomaly = 2.0 * PI - true_anomaly;
        }

        eccentric_anomaly = convert_true_to_eccentric_anomaly(true_anomaly, eccentricity);
        mean_anomaly_initial = mean_anomaly = eccentric_anomaly - eccentricity * sin(eccentric_anomaly);
    }
    else if (eccentricity > 1.0)
    {
        orbit_compression_ratio = eccentricity * eccentricity - 1;
        semi_major_axis = focal_parameter / orbit_compression_ratio;
        semi_minor_axis = semi_major_axis * sqrt(orbit_compression_ratio);
        center_point = eccentricity_vector * semi_major_axis;
        period = std::numeric_limits<double>::max();
        mean_motion = sqrt(mu / pow(semi_major_axis, 3));
        apoapsis = vec3d(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
        periapsis = center_point - semi_major_axis_basis * (semi_major_axis);
        periapsis_distance = periapsis.length();
        apoapsis_distance = std::numeric_limits<double>::max();
        true_anomaly = position.angle(eccentricity_vector);
        if (position.cross(-semi_major_axis_basis).dot(orbit_normal) < 0)
        {
            true_anomaly = -true_anomaly;
        }

        eccentric_anomaly = convert_true_to_eccentric_anomaly(true_anomaly, eccentricity);
        mean_anomaly_initial = mean_anomaly = sinh(eccentric_anomaly) * eccentricity - eccentric_anomaly;
    }
}

double kepler_orbit::convert_eccentric_to_true_anomaly(double eccentric_anomaly, double eccentricity)
{
    if (eccentricity < 1.0)
    {
        double cosE = cos(eccentric_anomaly);
        double tAnom = acos((cosE - eccentricity) / (1.0 - eccentricity * cosE));
        if (eccentric_anomaly > PI)
        {
            tAnom = 2.0 * PI - tAnom;
        }

        return tAnom;
    }
    else if (eccentricity > 1.0)
    {
        double tAnom = atan2(sqrt(eccentricity * eccentricity - 1.0) * sinh(eccentric_anomaly), eccentricity - cosh(eccentric_anomaly));
        return tAnom;
    }
    else
    {
        return eccentric_anomaly;
    }
}

double kepler_orbit::convert_true_to_eccentric_anomaly(double true_anomaly, double eccentricity)
{
    if (eccentricity != eccentricity || eccentricity == std::numeric_limits<double>::max())
    {
        return true_anomaly;
    }

    true_anomaly = fmod(true_anomaly, 2.0 * PI);
    if (eccentricity < 1.0)
    {
        if (true_anomaly < 0)
        {
            true_anomaly += 2.0 * PI;
        }

        double cosT2 = cos(true_anomaly);
        double eccAnom = acos((eccentricity + cosT2) / (1.0 + eccentricity * cosT2));
        if (true_anomaly > PI)
        {
            eccAnom = 2.0 * PI - eccAnom;
        }

        return eccAnom;
    }
    else if (eccentricity > 1.0)
    {
        double cosT = cos(true_anomaly);
        double eccAnom = acosh((eccentricity + cosT) / (1.0 + eccentricity * cosT)) * sign(true_anomaly);
        return eccAnom;
    }
    else
    {
        // For parabolic trajectories
        // there is no Eccentric anomaly defined,
        // because 'True anomaly' to 'Time' relation can be resolved analytically.
        return true_anomaly;
    }
}

// https://github.com/Karth42/SimpleKeplerOrbits/blob/master/Assets/SimpleKeplerOrbits/Scripts/Utils/KeplerOrbitUtils.cs#L256

double kepler_orbit::kepler_solver_ellipse(double mean_anomaly, double eccentricity)
{
    // Iterations count range from 2 to 6 when eccentricity is in range from 0 to 1.
    int iterations = (int)(std::ceil((eccentricity + 0.7) * 1.25)) << 1;
    double m = mean_anomaly;
    double esinE;
    double ecosE;
    double deltaE;
    double n;
    for (int i = 0; i < iterations; i++)
    {
        esinE = eccentricity * std::sin(m);
        ecosE = eccentricity * std::cos(m);
        deltaE = m - esinE - mean_anomaly;
        n = 1.0 - ecosE;
        m += -5.0 * deltaE / (n + sign(n) * std::sqrt(std::abs(16.0 * n * n - 20.0 * deltaE * esinE)));
    }

    return m;
}

// https://github.com/Karth42/SimpleKeplerOrbits/blob/master/Assets/SimpleKeplerOrbits/Scripts/Utils/KeplerOrbitUtils.cs#L283

double kepler_orbit::kepler_solver_hyperbola(double meanAnomaly, double eccentricity)
{
    double delta = 1.0;

    // Danby guess.
    double F = std::log(2.0 * std::abs(meanAnomaly) / eccentricity + 1.8);
    if (F != F)
    {
        return meanAnomaly;
    }

    while (delta > 1e-8 || delta < -1e-8)
    {
        delta = (eccentricity * std::sinh(F) - F - meanAnomaly) / (eccentricity * std::cosh(F) - 1.0);
        F -= delta;
    }

    return F;
}

double kepler_orbit::convert_mean_to_eccentric_anomaly(double mean_anomaly, double eccentricity)
{
    if (eccentricity < 1.0)
    {
        return kepler_solver_ellipse(mean_anomaly, eccentricity);
    }
    else if (eccentricity > 1.0)
    {
        return kepler_solver_hyperbola(mean_anomaly, eccentricity);
    }
    else
    {
        double m = mean_anomaly * 2;
        double v = 12.0 * m + 4.0 * sqrt(4.0 + 9.0 * m * m);
        double p = pow(v, 1.0 / 3.0);
        double t = 0.5 * p - 2.0 / p;
        return 2.0 * atan(t);
    }
}

vec3d kepler_orbit::get_central_position_at_eccentric_anomaly(double eccentric_anomaly)
{
    if (eccentricity < 1.0)
    {
        vec3d result = vec2d(sin(eccentric_anomaly) * semi_minor_axis, -cos(eccentric_anomaly) * semi_major_axis);
        return -semi_minor_axis_basis * result.x - semi_major_axis_basis * result.y;
    }
    else if (eccentricity > 1.0)
    {
        vec3d result = vec2d(sinh(eccentric_anomaly) * semi_minor_axis, cosh(eccentric_anomaly) * semi_major_axis);
        return -semi_minor_axis_basis * result.x - semi_major_axis_basis * result.y;
    }
    else
    {
        vec3d pos = vec2d(
            periapsis_distance * sin(eccentric_anomaly) / (1.0 + cos(eccentric_anomaly)),
            periapsis_distance * cos(eccentric_anomaly) / (1.0 + cos(eccentric_anomaly)));
        return -semi_minor_axis_basis * pos.x + semi_major_axis_basis * pos.y;
    }
}

vec3d kepler_orbit::get_velocity_at_true_anomaly(double trueAnomaly)
{
    if (focal_parameter <= 0)
    {
        return {};
    }

    double sqrtMGdivP = sqrt(attractor_mass * gravitational_constant / focal_parameter);
    double vX = sqrtMGdivP * (eccentricity + cos(trueAnomaly));
    double vY = sqrtMGdivP * sin(trueAnomaly);
    return -semi_minor_axis_basis * vX - semi_major_axis_basis * vY;
}

vec3d kepler_orbit::get_central_position_at_true_anomaly(double true_anomaly)
{
    double ecc = convert_true_to_eccentric_anomaly(true_anomaly, eccentricity);
    return get_central_position_at_eccentric_anomaly(ecc);
}

vec3d kepler_orbit::get_focal_position_at_eccentric_anomaly(double eccentric_anomaly)
{
    return get_central_position_at_eccentric_anomaly(eccentric_anomaly) + center_point;
}

vec3d kepler_orbit::get_focal_position_at_true_anomaly(double true_anomaly)
{
    return get_central_position_at_true_anomaly(true_anomaly) + center_point;
}

void kepler_orbit::set_position_by_current_anomaly()
{
    position = get_focal_position_at_eccentric_anomaly(eccentric_anomaly);
}

vec3d kepler_orbit::get_velocity_at_eccentric_anomaly(double eccentric_anomaly)
{
    return get_velocity_at_true_anomaly(convert_eccentric_to_true_anomaly(eccentric_anomaly, eccentricity));
}

void kepler_orbit::set_velocity_by_current_anomaly()
{
    velocity = get_velocity_at_eccentric_anomaly(eccentric_anomaly);
}

void kepler_orbit::process_updated_mean_anomaly()
{
    if (eccentricity < 1.0)
    {
        mean_anomaly = fmod(mean_anomaly, 2.0 * PI);

        if (mean_anomaly < 0.0)
        {
            mean_anomaly = 2.0 * PI - mean_anomaly;
        }

        eccentric_anomaly = kepler_solver_ellipse(mean_anomaly, eccentricity);
        double cosE = cos(eccentric_anomaly);
        true_anomaly = acos((cosE - eccentricity) / (1 - eccentricity * cosE));
        if (mean_anomaly > PI)
        {
            true_anomaly = 2.0 * PI - true_anomaly;
        }
    }
    else if (eccentricity > 1.0)
    {
        eccentric_anomaly = kepler_solver_hyperbola(mean_anomaly, eccentricity);
        true_anomaly = atan2(sqrt(eccentricity * eccentricity - 1.0) * sinh(eccentric_anomaly), eccentricity - cosh(eccentric_anomaly));
    }
    else
    {
        eccentric_anomaly = convert_mean_to_eccentric_anomaly(mean_anomaly, eccentricity);
        true_anomaly = eccentric_anomaly;
    }
}
