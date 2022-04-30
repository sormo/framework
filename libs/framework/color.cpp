#include "color.h"

Color Color::RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    Color result;
    result.data = nvgRGBA(r, g, b, a);
    return result;
}

Color Color::RGBf(float r, float g, float b, float a)
{
    Color result;
    result.data = nvgRGBAf(r, g, b, a);
    return result;
}

Color Color::HSL(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    Color result;
    result.data = nvgHSLA(r / 255.0f, g/ 255.0f, b/ 255.0f, a);
    return result;
}

Color Color::HSLf(float r, float g, float b, float a)
{
    Color result;
    result.data = nvgHSLA(r, g, b, a * 255);
    return result;
}

Color Color::Lerp(const Color& destination, float time) const
{
    Color result;
    result.data = nvgLerpRGBA(data, destination.data, time);
    return result;
}

Color Color::Transparency(uint8_t value) const
{
    Color result;
    result.data = data;
    result.data.a = value / 255.0f;
    return result;
}

float& Color::Alpha()
{
    return data.a;
}

float& Color::Red()
{
    return data.r;
}

float& Color::Green()
{
    return data.g;
}

float& Color::Blue()
{
    return data.b;
}

const Color Color::BLANK = Color::RGB(0, 0, 0, 0);
const Color Color::WHITE = Color::RGB(255, 255, 255);
const Color Color::LIGHTGRAY = Color::RGB(200, 200, 200);
const Color Color::GRAY = Color::RGB(100, 100, 100);
const Color Color::DARKGRAY = Color::RGB(80, 80, 80);
const Color Color::YELLOW = Color::RGB(253, 249, 0);
const Color Color::GOLD = Color::RGB(255, 203, 0);
const Color Color::ORANGE = Color::RGB(255, 161, 0);
