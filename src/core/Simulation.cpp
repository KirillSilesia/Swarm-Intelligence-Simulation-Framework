#include "Simulation.h"
#include "../scenarios/Reconnaissance.h"
#include "../scenarios/ObstacleAvoidance.h"
#include <algorithm>
#include <cmath>
#include <numeric>

void Simulation::start(std::shared_ptr<SwarmAlgorithm> algorithm,
    std::shared_ptr<Scenario>       scenario,
    int agentCount)
{
    m_algorithm = algorithm;
    m_scenario = scenario;
    m_running = true;
    m_hasFinished = false;
    m_elapsed = 0.0f;
    m_sampleTimer = 0.0f;

    m_agents.resize(agentCount);
    m_scenario->reset(m_agents);
    m_algorithm->initialize(m_agents, *m_scenario);

    // Initialise result header
    m_result = SimulationResult{};
    m_result.algorithmName = algorithm->getName();
    m_result.scenarioName = scenario->getName();
    m_result.agentCount = agentCount;
    m_result.completed = false;
}

void Simulation::update(float dt) {
    if (!m_running || m_hasFinished) return;

    m_elapsed += dt;
    m_sampleTimer += dt;

    m_algorithm->update(m_agents, *m_scenario, dt);
    m_scenario->update(dt, m_agents);

    // Sample report data every ~0.25 s
    if (m_sampleTimer >= 0.25f) {
        sampleReportFrame();
        m_sampleTimer = 0.0f;
    }

    if (m_scenario->isFinished()) {
        m_hasFinished = true;
        m_running = false;
        finaliseResult();
    }
}

void Simulation::sampleReportFrame() {
    int alive = 0, atGoal = 0;
    float sumSpeed = 0.0f, sumX = 0.0f, sumY = 0.0f;
    for (const auto& a : m_agents) {
        if (a.alive) {
            ++alive;
            sumSpeed += std::hypot(a.vx, a.vy);
            sumX += a.x; sumY += a.y;
        }
        if (a.atGoal) ++atGoal;
    }
    float avgX = alive > 0 ? sumX / alive : 0.5f;
    float avgY = alive > 0 ? sumY / alive : 0.5f;

    // Standard deviation of positions (diversity)
    float varSum = 0.0f;
    for (const auto& a : m_agents)
        if (a.alive)
            varSum += (a.x - avgX) * (a.x - avgX) + (a.y - avgY) * (a.y - avgY);
    float diversity = alive > 0 ? std::sqrt(varSum / alive) : 0.0f;

    // Best fitness among living agents
    float bestF = 1e9f;
    for (const auto& a : m_agents)
        if (a.alive) {
            float f = m_scenario->evaluateFitness(a.x, a.y);
            if (f < bestF) bestF = f;
        }

    ReportFrame fr;
    fr.time = m_elapsed;
    fr.bestFitness = bestF < 1e8f ? bestF : 0.0f;
    fr.aliveAgents = alive;
    fr.atGoalAgents = atGoal;
    fr.avgSpeed = alive > 0 ? sumSpeed / alive : 0.0f;
    fr.diversity = diversity;
    m_result.frames.push_back(fr);
}

void Simulation::finaliseResult() {
    m_result.completionTime = m_elapsed;
    m_result.completed = true;
    m_result.agentsAtGoal = m_scenario->agentsAtGoal;

    int alive = 0;
    for (const auto& a : m_agents) if (a.alive) ++alive;
    m_result.finalAliveAgents = alive;

    // Reconnaissance extras
    if (auto* r = dynamic_cast<Reconnaissance*>(m_scenario.get())) {
        m_result.highPriorityFound = r->highFound();
        m_result.mediumPriorityFound = r->medFound();
        m_result.lowPriorityFound = r->lowFound();
    }
}

void Simulation::drawOverlay() const {
    // Called after scenario.draw() to render algorithm-specific overlays
    // We need the rendering bounds; use approximate values based on display
    // (exact bounds are computed inside the scenario draw, so we approximate)
    if (m_algorithm)
        m_algorithm->drawOverlay(0.0f, 1.0f, 0.0f, 500.0f);
}