#pragma once
#include "../core/Agent.h"
#include "../scenarios/Scenario.h"
#include <vector>

class SwarmAlgorithm {
public:
    virtual ~SwarmAlgorithm() = default;

    virtual const char* getName() const = 0;

    virtual void initialize(std::vector<Agent>& agents,
        Scenario& scenario) = 0;

    virtual void update(std::vector<Agent>& agents,
        Scenario& scenario,
        float dt) = 0;

    // Draw an algorithm-specific overlay aligned to the scenario's play area.
    // Read scenario.viewX/viewY/viewW/viewH to map normalized [0,1] coordinates
    // onto exactly the on-screen rectangle the scenario drew itself into.
    virtual void drawOverlay(const Scenario& /*scenario*/) {}
};