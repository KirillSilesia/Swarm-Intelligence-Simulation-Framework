#pragma once
#include "Scenario.h"

class TargetSearchScenario : public Scenario {
    bool found = false;
public:
    const char* getName() const override;
    void reset() override;
    void update(float deltaTime) override;
    bool isFinished() const override;
};
