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

    void LerpColor(NVGcolor& color, const NVGcolor& destination, FinishedCallback finished = nullptr);
    void LerpObject(World::Object object, const NVGcolor& destination, FinishedCallback finished = nullptr);
    void Update();

private:
    struct Lerper
    {
        Lerper(const NVGcolor& source, const NVGcolor& destination);
        virtual ~Lerper() = default;
        virtual bool Update();
        NVGcolor CurrentColor();
        NVGcolor source;
        NVGcolor destination;
        FinishedCallback finished;
        float time = 0.0f;
    };

    struct LerperColor;
    struct LerperObject;

    std::vector<std::unique_ptr<Lerper>> m_lerpers;
};
