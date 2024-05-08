#include "color_type.h"

color_type color_type::RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    color_type result;
    result.data = nvgRGBA(r, g, b, a);
    return result;
}

color_type color_type::RGBf(float r, float g, float b, float a)
{
    color_type result;
    result.data = nvgRGBAf(r, g, b, a);
    return result;
}

color_type color_type::HSL(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    color_type result;
    result.data = nvgHSLA(r / 255.0f, g/ 255.0f, b/ 255.0f, a);
    return result;
}

color_type color_type::HSLf(float r, float g, float b, float a)
{
    color_type result;
    result.data = nvgHSLA(r, g, b, a * 255);
    return result;
}

color_type color_type::HEX(uint32_t number)
{
    uint8_t alpha = 255;
    if (number > 0xffffff)
        alpha = number & 0xff;

    return RGB((number >> 24) & 0xff, (number >> 16) & 0xff, (number >> 8) & 0xff, alpha);
}

color_type color_type::lerp(const color_type& destination, float time) const
{
    color_type result;
    result.data = nvgLerpRGBA(data, destination.data, time);
    return result;
}

color_type color_type::transparency(uint8_t value) const
{
    color_type result;
    result.data = data;
    result.data.a = value / 255.0f;
    return result;
}

float& color_type::alpha()
{
    return data.a;
}

float& color_type::red()
{
    return data.r;
}

float& color_type::green()
{
    return data.g;
}

float& color_type::blue()
{
    return data.b;
}

float color_type::alpha() const
{
    return data.a;
}

float color_type::red() const
{
    return data.r;
}

float color_type::green() const
{
    return data.g;
}

float color_type::blue() const
{
    return data.b;
}

const color_type color_type::BLANK = color_type::RGB(0, 0, 0, 0);
const color_type color_type::WHITE = color_type::RGB(255, 255, 255);
const color_type color_type::BLACK = color_type::RGB(0, 0, 0);
const color_type color_type::LIGHTGRAY = color_type::RGB(200, 200, 200);
const color_type color_type::GRAY = color_type::RGB(100, 100, 100);
const color_type color_type::DARKGRAY = color_type::RGB(80, 80, 80);
const color_type color_type::YELLOW = color_type::RGB(253, 249, 0);
const color_type color_type::GOLD = color_type::RGB(255, 203, 0);
const color_type color_type::ORANGE = color_type::RGB(255, 161, 0);
const color_type color_type::EARTHBLUE = color_type::RGB(137, 173, 182);
const color_type color_type::RED = color_type::RGB(255, 0, 0);
const color_type color_type::GREEN = color_type::RGB(0, 255, 0);
const color_type color_type::BLUE = color_type::RGB(0, 0, 255);
