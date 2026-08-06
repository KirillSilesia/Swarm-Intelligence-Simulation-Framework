#include "Simulation.h"
#include "../scenarios/Reconnaissance.h"
#include "../scenarios/ObstacleAvoidance.h"
#include "../scenarios/PathPlanning.h"
#include "../algorithms/AntColonyOptimization.h"
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

    m_result = SimulationResult{};
    m_result.algorithmName = algorithm->getName();
    m_result.scenarioName = scenario->getName();
    m_result.agentCount = agentCount;
    m_result.completed = false;
}

void Simulation::update(float dt) {
    if (!m_running || m_hasFinished) return;

    // Advance in fixed-size sub-steps. The algorithms move agents by
    // velocity*dt and only test walls at the agent's current point, so a large
    // dt (from the Speed multiplier) makes them leap a whole cell per tick,
    // overshoot into walls and jam -- the maze looked "frozen" at high speed for
    // exactly this reason. Splitting the frame into small steps keeps every move
    // stable, so behaviour is the same at 1x or 10x, just faster.
    const float MAX_STEP = 0.016f;
    int steps = (int)std::ceil(dt / MAX_STEP);
    if (steps < 1) steps = 1;
    float sub = dt / steps;

    for (int i = 0; i < steps; ++i) {
        m_elapsed += sub;
        m_sampleTimer += sub;

        m_algorithm->update(m_agents, *m_scenario, sub);
        m_scenario->update(sub, m_agents);

        if (m_sampleTimer >= 0.25f) {
            sampleReportFrame();
            m_sampleTimer = 0.0f;
        }

        if (m_scenario->isFinished()) {
            m_hasFinished = true;
            m_running = false;
            finaliseResult();
            break;
        }
    }
}

void Simulation::stop() {
    if (!m_running) return;
    m_hasFinished = true;
    m_running = false;
    finaliseResult();
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

    float varSum = 0.0f;
    for (const auto& a : m_agents)
        if (a.alive)
            varSum += (a.x - avgX) * (a.x - avgX) + (a.y - avgY) * (a.y - avgY);
    float diversity = alive > 0 ? std::sqrt(varSum / alive) : 0.0f;

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

    if (auto* r = dynamic_cast<Reconnaissance*>(m_scenario.get())) {
        m_result.highPriorityFound = r->highFound();
        m_result.mediumPriorityFound = r->medFound();
        m_result.lowPriorityFound = r->lowFound();
    }
}