#pragma once
#include <memory>
#include <vector>
#include "Agent.h"
#include "Scenario.h"
#include "SwarmAlgorithm.h"

class Simulation {
public:
    void start(std::shared_ptr<SwarmAlgorithm> algorithm,
        std::shared_ptr<Scenario> scenario,
        int agentCount);

    void update(float deltaTime);
    bool isFinished() const;
    bool isRunning() const { return m_scenario != nullptr && m_algorithm != nullptr; }

private:
    std::vector<Agent> m_agents;
    std::shared_ptr<Scenario> m_scenario;
    std::shared_ptr<SwarmAlgorithm> m_algorithm;
};