#pragma once

struct unit
{
    static double second;
    static double minute;
    static double hour;
    static double day;
    static double month;
    static double year;

    // mass
    static double kilogram;
    static double ton;
    static double megaton;

    // length
    static double meter;
    static double kilometer;
    static double AU;

    static double GRAVITATIONAL_CONSTANT;

    static void set_base_second(double base);
    static void set_base_kilogram(double base);
    static void set_base_meter(double base);
};