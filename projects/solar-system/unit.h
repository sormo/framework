#pragma once

namespace unit
{
    double second = 1.0;
    double minute = 60.0 * second;
    double hour = 60.0 * minute;
    double day = 24.0 * hour;
    double month = 31.0 * day;
    double year = 365 * day;

    // mass
    double kilogram = 1.0;
    double ton = 1e3 * kilogram;
    double megaton = 1e6 * ton;

    // length
    double meter = 1.0;
    double kilometer = 1e3 * meter;
    double AU = 149597870700 * meter;

    void set_base_second(double base)
    {
        second = base;
        minute = 60.0 * second;
        hour = 60.0 * minute;
        day = 24.0 * hour;
        month = 31.0 * day;
        year = 365 * day;
    }

    void set_base_kilogram(double base)
    {
        kilogram = base;
        ton = 1e3 * kilogram;
        megaton = 1e6 * ton;
    }

    void set_base_meter(double base)
    {
        meter = base;
        kilometer = 1e3 * meter;
        AU = 149597870700 * meter;
    }
}