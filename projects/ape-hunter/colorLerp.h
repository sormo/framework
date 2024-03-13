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

    void LerpColor(frame::col4& color, const frame::col4& destination, FinishedCallback finished = nullptr);
    void LerpObject(World::Object object, const frame::col4& destination, FinishedCallback finished = nullptr);
    void Update();

private:
    struct Lerper
    {
        Lerper(const frame::col4& source, const frame::col4& destination);
        virtual ~Lerper() = default;
        virtual bool Update();
        frame::col4 CurrentColor();
        frame::col4 source;
        frame::col4 destination;
        FinishedCallback finished;
        float time = 0.0f;
    };

    struct LerperColor;
    struct LerperObject;

    std::vector<std::unique_ptr<Lerper>> m_lerpers;
};
