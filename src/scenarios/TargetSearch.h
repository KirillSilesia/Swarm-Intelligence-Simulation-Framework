#pragma once
#include "Scenario.h"
#include "Agent.h"

class TargetSearch : public Scenario {
    bool found = false;
public:
    const char* getName() const override;
    void reset(std::vector<Agent> &agents) override;
    void update(float deltaTime, std::vector<Agent>& agents) override;
    bool isFinished() const override;
    void draw(const std::vector<Agent>& agents) override;
};
