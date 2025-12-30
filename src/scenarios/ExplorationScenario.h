#pragma once
#include "Scenario.h"
#include "../core/Agent.h"

class ExplorationScenario : public Scenario {
    bool finished = false;
public:
    const char* getName() const override;
    void reset() override;
    void update(float deltaTime) override;
    bool isFinished() const override;
};
