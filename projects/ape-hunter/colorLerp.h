#pragma once
#include "framework.h"
#include "world.h"
#include <vector>
#include <memory>
#include <functional>

class ColorLerp
{
public:
    using FinishedCallback = std::function<void()>;

    void LerpColor(Color& color, const Color& destination, FinishedCallback finished = nullptr);
    void LerpObject(World::Object object, const Color& destination, FinishedCallback finished = nullptr);
    void Update();

private:
    struct Lerper
    {
        Lerper(const Color& source, const Color& destination);
        virtual ~Lerper() = default;
        virtual bool Update();
        Color CurrentColor();
        Color source;
        Color destination;
        FinishedCallback finished;
        float time = 0.0f;
    };

    struct LerperColor;
    struct LerperObject;

    std::vector<std::unique_ptr<Lerper>> m_lerpers;
};
