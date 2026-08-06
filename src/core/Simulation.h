#pragma once
#include <memory>
#include <vector>
#include "Agent.h"
#include "../scenarios/Scenario.h"
#include "../algorithms/SwarmAlgorithm.h"
#include "SimulationResult.h"

class Simulation {
public:
    void start(std::shared_ptr<SwarmAlgorithm> algorithm,
        std::shared_ptr<Scenario>       scenario,
        int agentCount);

    void update(float dt);

    // Stop a running simulation early, exactly as if the scenario had finished:
    // freezes updates and finalises the result so a report can be viewed.
    void stop();

    bool isRunning()  const { return m_running; }
    bool hasFinished() const { return m_hasFinished; }

    const std::vector<Agent>& getAgents()    const { return m_agents; }
    const SimulationResult& getResult()    const { return m_result; }
    const std::shared_ptr<Scenario>& getScenario()  const { return m_scenario; }
    const std::shared_ptr<SwarmAlgorithm>& getAlgorithm() const { return m_algorithm; }

private:
    std::vector<Agent>             m_agents;
    std::shared_ptr<Scenario>      m_scenario;
    std::shared_ptr<SwarmAlgorithm> m_algorithm;
    SimulationResult               m_result;

    bool  m_running = false;
    bool  m_hasFinished = false;
    float m_elapsed = 0.0f;
    float m_sampleTimer = 0.0f;

    void sampleReportFrame();
    void finaliseResult();
};