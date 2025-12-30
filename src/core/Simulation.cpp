#include "Simulation.h"

void Simulation::start(
    std::shared_ptr<SwarmAlgorithm> algorithm,
    std::shared_ptr<Scenario> scenario,
    int agentCount
) {
    m_algorithm = algorithm;
    m_scenario = scenario;
    m_agents.resize(agentCount);

    m_algorithm->initialize(m_agents, *m_scenario);
    m_scenario->reset();
}

void Simulation::update(float deltaTime) {
    if (!isFinished()) {
        m_algorithm->update(m_agents, *m_scenario, deltaTime);
        m_scenario->update(deltaTime);
    }
}

bool Simulation::isFinished() const {
    return m_scenario->isFinished();
}
