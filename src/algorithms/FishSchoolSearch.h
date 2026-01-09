#pragma once
#include "SwarmAlgorithm.h"
#include "../core/Agent.h"
#include "../scenarios/Scenario.h"
#include <vector>

class FishSchoolSearch : public SwarmAlgorithm {
public:
    const char* getName() const override;
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
};
