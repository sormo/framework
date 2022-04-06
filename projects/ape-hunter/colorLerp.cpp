#include "colorLerp.h"

float LerpTimeIncrement = 0.05f;

extern World world;

ColorLerp::Lerper::Lerper(const Color& source, const Color& destination)
    : source(source), destination(destination)
{
}
bool ColorLerp::Lerper::Update()
{
    time += LerpTimeIncrement;
    return time < 1.0f;
}
Color ColorLerp::Lerper::CurrentColor()
{
    return source.Lerp(destination, time);
}

struct ColorLerp::LerperColor : public ColorLerp::Lerper
{
    LerperColor(Color& color, const Color& destination)
        : Lerper(color, destination), color(color)
    {
    }
    virtual bool Update() override
    {
        color = CurrentColor();

        return Lerper::Update();
    }
    Color& color;
};

struct ColorLerp::LerperObject : public ColorLerp::Lerper
{
    LerperObject(World::Object object, const Color& destination)
        : Lerper(world.GetFill(object), destination), object(object)
    {
    }
    virtual bool Update() override
    {
        world.SetFill(object, CurrentColor());
        return Lerper::Update();
    }
    World::Object object;
};

void ColorLerp::LerpColor(Color& color, const Color& destination, FinishedCallback finished)
{
    m_lerpers.emplace_back(std::make_unique<LerperColor>(color, destination));
    m_lerpers.back()->finished = finished;
}

void ColorLerp::LerpObject(World::Object object, const Color& destination, FinishedCallback finished)
{
    m_lerpers.emplace_back(std::make_unique<LerperObject>(object, destination));
    m_lerpers.back()->finished = finished;
}

void ColorLerp::Update()
{
    std::vector<FinishedCallback> callbacks;

    for (auto it = std::begin(m_lerpers); it != std::end(m_lerpers);)
    {
        if (!it->get()->Update())
        {
            if (it->get()->finished)
                callbacks.push_back(it->get()->finished);

            it = m_lerpers.erase(it);
        }
        else
        {
            it++;
        }
    }

    // flush callbacks after update
    for (auto& c : callbacks)
        c();
}
