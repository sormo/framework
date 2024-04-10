#pragma once
#include <nanovg.h>
#include <stdint.h>

#ifdef RGB
#undef RGB
#endif

struct color_type
{
    static color_type RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    static color_type RGBf(float r, float g, float b, float a = 1.0f);
    static color_type HSL(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    static color_type HSLf(float r, float g, float b, float a = 1.0f);

    color_type lerp(const color_type& destination, float time) const;
    color_type transparency(uint8_t value) const;

    float& red();
    float& green();
    float& blue();
    float& alpha();

    float red() const;
    float green() const;
    float blue() const;
    float alpha() const;

    static const color_type BLANK;           // Blank (Transparent)
    static const color_type WHITE;
    static const color_type BLACK;
    static const color_type LIGHTGRAY;       // Light Gray
    static const color_type GRAY;            // Gray
    static const color_type DARKGRAY;        // Dark Gray
    static const color_type YELLOW;          // Yellow
    static const color_type GOLD;            // Gold
    static const color_type ORANGE;          // Orange
    static const color_type EARTHBLUE;
    static const color_type RED;
    static const color_type GREEN;
    static const color_type BLUE;

    NVGcolor data;
};
