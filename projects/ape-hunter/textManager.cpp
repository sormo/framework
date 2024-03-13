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

void TextManager::SetPosition(Handle text, const frame::vec2& position)
{
    m_texts[text].position = position;
}

void TextManager::SetSize(Handle text, float size)
{
    m_texts[text].size = size;
}

void TextManager::SetColor(Handle text, const frame::col4& color)
{
    m_texts[text].color = color;
}

void TextManager::SetHidden(Handle text, bool hidden)
{
    m_texts[text].isHidden = hidden;
}

void TextManager::FadeIn(Handle text)
{
    auto currentColor = m_texts[text].color;

    m_texts[text].isHidden = false;
    m_texts[text].color.alpha() = 0.0f;
    m_lerp.LerpColor(m_texts[text].color, currentColor);
}

void TextManager::FadeOut(Handle text)
{
    auto transparentColor = m_texts[text].color.transparency(0);

    m_texts[text].isHidden = false;
    m_lerp.LerpColor(m_texts[text].color, transparentColor,
    [this, currentAlpha = m_texts[text].color.alpha(), text]()
    {
        m_texts[text].isHidden = true;
        m_texts[text].color.alpha() = currentAlpha;
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

        frame::draw_text(data.text.c_str(), data.position, data.size, data.color);
    }
}

void TextManager::Draw(Handle text)
{
    Data& data = m_texts[text];
    if (data.isHidden)
        return;

    frame::draw_text(data.text.c_str(), data.position, data.size, data.color);
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

Text& Text::SetPosition(const frame::vec2& position)
{
    m_manager->SetPosition(m_handle, position);
    return *this;
}

Text& Text::SetSize(float size)
{
    m_manager->SetSize(m_handle, size);
    return *this;
}

Text& Text::SetColor(const frame::col4& color)
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
