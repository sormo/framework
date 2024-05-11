#include "unit.h"

double unit::second = 1.0;
double unit::minute = 60.0 * second;
double unit::hour = 60.0 * minute;
double unit::day = 24.0 * hour;
double unit::month = 31.0 * day;
double unit::year = 365 * day;

// mass
double unit::kilogram = 1.0;
double unit::ton = 1e3 * kilogram;
double unit::megaton = 1e6 * ton;

// length
double unit::meter = 1.0;
double unit::kilometer = 1e3 * meter;
double unit::AU = 149597870700 * meter;

double unit::GRAVITATIONAL_CONSTANT = 0.8;

void unit::set_base_second(double base)
{
    second = base;
    minute = 60.0 * second;
    hour = 60.0 * minute;
    day = 24.0 * hour;
    month = 31.0 * day;
    year = 365 * day;
}

void unit::set_base_kilogram(double base)
{
    kilogram = base;
    ton = 1e3 * kilogram;
    megaton = 1e6 * ton;
}

void unit::set_base_meter(double base)
{
    meter = base;
    kilometer = 1e3 * meter;
    AU = 149597870700 * meter;
}
