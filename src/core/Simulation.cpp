#include "Simulation.h"
#include  "Agent.h"

void Simulation::start(
    std::shared_ptr<SwarmAlgorithm> algorithm,
    std::shared_ptr<Scenario> scenario,
    int agentCount
) {
    m_algorithm = algorithm;
    m_scenario = scenario;
    m_agents.resize(agentCount);

    m_scenario->reset(m_agents);
    m_algorithm->initialize(m_agents, *m_scenario);
}

void Simulation::update(float deltaTime) {
    if (!isFinished()) {
        m_algorithm->update(m_agents, *m_scenario, deltaTime);
        m_scenario->update(deltaTime, m_agents);
    }
}

bool Simulation::isFinished() const {
    return m_scenario->isFinished();
}
