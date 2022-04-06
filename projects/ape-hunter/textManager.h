#pragma once
#include <framework.h>
#include <string>
#include <unordered_map>
#include "colorLerp.h"

class TextManager;

class Text
{
public:
    Text& SetPosition(const Point& position);
    Text& SetWidth(float width);
    Text& SetSize(float size);
    Text& SetColor(const Color& color);
    Text& SetHidden(bool hidden);

    Text& FadeIn();
    Text& FadeOut();

    bool GetHidden();

    Text() = default;
    Text(const Text& o);
    Text& operator=(const Text& o);

private:
    Text(uint32_t handle, TextManager* manager);
    uint32_t m_handle = 0;
    TextManager* m_manager = nullptr;
    friend class TextManager;
};

class TextManager
{
public:
    TextManager(ColorLerp& lerper);

    Text Text(const char* text);

    using Handle = int32_t;

    TextManager::Handle Create(const char* text);
    void SetPosition(Handle text, const Point& position);
    void SetWidth(Handle text, float width);
    void SetSize(Handle text, float size);
    void SetColor(Handle text, const Color& color);
    void SetHidden(Handle text, bool hidden);

    void FadeIn(Handle text);
    void FadeOut(Handle text);

    bool GetHidden(Handle text);

    // draw all texts
    void Draw();
    void Draw(Handle text);

private:
    struct Data
    {
        std::string text;
        Point position;
        float width = std::numeric_limits<float>::max();
        float size = 12.0f;
        Color color = Color::WHITE;
        bool isHidden = false;
    };

    Handle m_counter = 1;
    std::unordered_map<Handle, Data> m_texts;
    ColorLerp& m_lerp;
};
