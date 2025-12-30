#pragma once
#include "SwarmAlgorithm.h"
#include "../core/Agent.h" 
#include "../scenarios/Scenario.h" 
#include <vector>
#include <cstdlib>
#include <ctime>

/**
 * Ant Colony Optimization (ACO) algorithm implementation.
 */
class AntColonyOptimization : public SwarmAlgorithm {
public:
    AntColonyOptimization() = default;

    const char* getName() const override;

    void initialize(
        std::vector<Agent>& agents,
        Scenario& scenario
    ) override;

    void update(
        std::vector<Agent>& agents,
        Scenario& scenario,
        float deltaTime
    ) override;
};
