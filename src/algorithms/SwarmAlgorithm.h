#pragma once
#include "../core/Agent.h" 
#include "../scenarios/Scenario.h" 
#include <vector>
#include <cstdlib>
#include <ctime>

class Scenario;

class SwarmAlgorithm {
public:
    virtual ~SwarmAlgorithm() = default;

    virtual const char* getName() const = 0;
    virtual void initialize(
        std::vector<Agent>& agents,
        Scenario& scenario
    ) = 0;

    virtual void update(
        std::vector<Agent>& agents,
        Scenario& scenario,
        float deltaTime
    ) = 0;
};
