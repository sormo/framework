#include "textManager.h"

Text TextManager::Text(const char* text)
{
    return { (uint32_t)Create(text), this };
}

TextManager::TextManager(ColorLerp& lerper)
    : m_lerp(lerper)
{

}

TextManager::Handle TextManager::Create(const char* text)
{
    Data data;
    data.text = text;

    Handle handle = m_counter++;
    m_texts.insert({handle, data});

    return handle;
}

void TextManager::SetPosition(Handle text, const Point& position)
{
    m_texts[text].position = position;
}

void TextManager::SetWidth(Handle text, float width)
{
    m_texts[text].width = width;
}

void TextManager::SetSize(Handle text, float size)
{
    m_texts[text].size = size;
}

void TextManager::SetColor(Handle text, const NVGcolor& color)
{
    m_texts[text].color = color;
}

void TextManager::SetHidden(Handle text, bool hidden)
{
    m_texts[text].isHidden = hidden;
}

void TextManager::FadeIn(Handle text)
{
    float alpha = m_texts[text].color.a;

    m_texts[text].isHidden = false;
    m_texts[text].color = nvgTransRGBAf(m_texts[text].color, 0.0f);
    m_lerp.LerpColor(m_texts[text].color, nvgTransRGBAf(m_texts[text].color, alpha));
}

void TextManager::FadeOut(Handle text)
{
    float alpha = m_texts[text].color.a;

    m_texts[text].isHidden = false;
    m_lerp.LerpColor(m_texts[text].color, nvgTransRGBAf(m_texts[text].color, 0.0f), [this, alpha, text]()
    {
        m_texts[text].isHidden = true;
        m_texts[text].color = nvgTransRGBAf(m_texts[text].color, alpha);
    });
}

bool TextManager::GetHidden(Handle text)
{
    return m_texts[text].isHidden;
}

void TextManager::Draw()
{
    for (const auto& [_, data] : m_texts)
    {
        if (data.isHidden)
            continue;

        frame::text(data.text.c_str(), data.position, data.width, data.size, data.color);
    }
}

void TextManager::Draw(Handle text)
{
    Data& data = m_texts[text];
    if (data.isHidden)
        return;

    frame::text(data.text.c_str(), data.position, data.width, data.size, data.color);
}

Text::Text(uint32_t handle, TextManager* manager)
    : m_handle(handle), m_manager(manager)
{
}

Text::Text(const Text& o)
    : m_handle(o.m_handle), m_manager(o.m_manager)
{
}

Text& Text::operator=(const Text& o)
{
    m_handle = o.m_handle;
    m_manager = o.m_manager;
    return *this;
}

Text& Text::SetPosition(const Point& position)
{
    m_manager->SetPosition(m_handle, position);
    return *this;
}

Text& Text::SetWidth(float width)
{
    m_manager->SetWidth(m_handle, width);
    return *this;
}

Text& Text::SetSize(float size)
{
    m_manager->SetSize(m_handle, size);
    return *this;
}

Text& Text::SetColor(const NVGcolor& color)
{
    m_manager->SetColor(m_handle, color);
    return *this;
}

Text& Text::SetHidden(bool hidden)
{
    m_manager->SetHidden(m_handle, hidden);
    return *this;
}

Text& Text::FadeIn()
{
    m_manager->FadeIn(m_handle);
    return *this;
}

Text& Text::FadeOut()
{
    m_manager->FadeOut(m_handle);
    return *this;
}

bool Text::GetHidden()
{
    return m_manager->GetHidden(m_handle);
}
