#pragma once
#include <framework.h>
#include <utils.h>

// https://github.com/Karth42/SimpleKeplerOrbits
struct kepler_orbit
{
    frame::vec3d position;
    frame::vec3d velocity;
    double attractor_mass = 0.0;
    double this_mass = 0.0;
    double gravitational_constant = 0.0;

    //

    double mu = 0.0;
    double attractor_distance = 0.0;

    // 

    double orbit_compression_ratio = 0.0;
    double semi_minor_axis = 0.0; // AU
    double semi_major_axis = 0.0; // AU
    double focal_parameter = 0.0;
    double eccentricity = 0.0;
    double period = 0.0;
    double true_anomaly = 0.0; // rad
    double mean_anomaly = 0.0; // rad
    double mean_anomaly_initial = 0.0; // rad
    double eccentric_anomaly = 0.0; // rad
    double mean_motion = 0.0;
    frame::vec3d periapsis;
    double periapsis_distance = 0.0;
    frame::vec3d apoapsis;
    double apoapsis_distance = 0.0;
    frame::vec3d center_point;
    frame::vec3d orbit_normal;
    frame::vec3d semi_minor_axis_basis;
    frame::vec3d semi_major_axis_basis;

    // if > 0, then orbit motion is clockwise
    double orbit_normal_dot_ecliptic_normal = 0.0;

    kepler_orbit();
    kepler_orbit(frame::vec3d pos, frame::vec3d vel, double attractor_mass, double this_mass, double gravitational_constant);

    void initialize(frame::vec3d pos, frame::vec3d vel, double attr_mass, double t_mass, double g_constant);
    void initialize(double eccentricity,
                    frame::vec3d semi_major_axis,
                    frame::vec3d semi_minor_axis,
                    double mean_anomaly_radians,
                    double attractor_mass,
                    double g_constant);
    void initialize(double eccentricity,
                    double semi_major_axis,
                    double mean_anomaly_rad,
                    double inclination_rad,
                    double arg_of_perifocus_rad,
                    double ascending_node_rad,
                    double attractor_mass,
                    double g_constant);

    std::vector<frame::vec3d> get_orbit_points(int points_count = 50, double max_distance = 1000.0);
    std::vector<frame::vec3d> get_orbit_points(int points_count, const frame::vec3d& origin, double max_distance = 1000.0);

    double get_current_orbit_time();
    void set_current_orbit_time(double time);
    void update_current_orbit_time_by_delta_time(double delta_time);

    static double compute_mass(double semi_major_axis, double period, double g_constant);

    void update_initial_mean_anomaly_with_time_offset(double time);

private:
    void calculate_orbit_state_from_orbital_elements();
    void calculate_initial_orbit_state();


    frame::vec3d get_central_position_at_eccentric_anomaly(double eccentric_anomaly);
    frame::vec3d get_velocity_at_true_anomaly(double trueAnomaly);
    frame::vec3d get_central_position_at_true_anomaly(double true_anomaly);
    frame::vec3d get_focal_position_at_eccentric_anomaly(double eccentric_anomaly);
    frame::vec3d get_focal_position_at_true_anomaly(double true_anomaly);
    void set_position_by_current_anomaly();
    frame::vec3d get_velocity_at_eccentric_anomaly(double eccentric_anomaly);

    // Sets orbit velocity, calculated by current anomaly.
    void set_velocity_by_current_anomaly();
    void process_updated_mean_anomaly();

    static double calc_true_anomaly_for_distance(double distance, double eccentricity, double semi_major_axis, double periapsis_distance);
    static double convert_eccentric_to_true_anomaly(double eccentric_anomaly, double eccentricity);
    static double convert_true_to_eccentric_anomaly(double true_anomaly, double eccentricity);
    static double kepler_solver_ellipse(double mean_anomaly, double eccentricity);
    static double kepler_solver_hyperbola(double meanAnomaly, double eccentricity);
    static double convert_mean_to_eccentric_anomaly(double mean_anomaly, double eccentricity);
};
