#pragma once
#include "Scenario.h"
#include <vector>

class TargetSearch : public Scenario {
private:
    bool m_found = false;

public:
    const char* getName() const override;

    void reset(std::vector<Agent>& agents) override;

    void update(float dt, std::vector<Agent>& agents) override;

    bool isFinished() const override;

    void draw(const std::vector<Agent>& agents,
        float xOffset, float ws) override;

    float evaluateFitness(float x, float y) const override;

    float getGoalX() const override;
    float getGoalY() const override;
};