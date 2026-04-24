#pragma once
#include <vector>
#include "Agent.h"

class Scenario {
public:
    virtual ~Scenario() = default;

    virtual const char* getName() const = 0;
    virtual void reset(std::vector<Agent>& agents) = 0;
    virtual void update(float deltaTime, std::vector<Agent>& agents) = 0;
    virtual bool isFinished() const = 0;
    virtual void draw(const std::vector<Agent>& agents, float xOffset, float widthScale) = 0;
};
