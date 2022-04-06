#pragma once
#include <nanovg.h>
#include <stdint.h>

#ifdef RGB
#undef RGB
#endif

struct Color
{
    static Color RGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    static Color RGBf(float r, float g, float b, float a = 1.0f);
    static Color HSL(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    static Color HSLf(float r, float g, float b, float a = 1.0f);

    Color Lerp(const Color& destination, float time) const;
    Color Transparency(uint8_t value) const;

    float& Red();
    float& Green();
    float& Blue();
    float& Alpha();

    static const Color BLANK;           // Blank (Transparent)
    static const Color WHITE;
    static const Color LIGHTGRAY;       // Light Gray
    static const Color GRAY;            // Gray
    static const Color DARKGRAY;        // Dark Gray
    static const Color YELLOW;          // Yellow
    static const Color GOLD;            // Gold
    static const Color ORANGE;          // Orange

    NVGcolor data;
};
