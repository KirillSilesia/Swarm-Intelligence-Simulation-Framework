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

    // Optional: draw algorithm-specific overlays (pheromones, best position…)
    virtual void drawOverlay(float /*xOffset*/, float /*widthScale*/,
        float /*yTop*/, float /*height*/) {
    }
};