/*
 * Artificial Bee Colony – per spec.
 *
 * Agents ARE food sources.  Each update frame is one ABC iteration:
 *
 *   1. Employed phase:
 *        Vi[j] = Xi[j] + φij * (Xi[j] - Xk[j])   φ∈[-1,1], k≠i
 *        If f(Vi) < f(Xi):  Xi ← Vi, trial[i]=0
 *        Else:              trial[i]++
 *
 *   2. Onlooker phase:
 *        Pi = fit[i] / Σfit[j]
 *        Each agent has a chance to explore based on Pi
 *        Same neighbourhood search as employed.
 *
 *   3. Scout phase:
 *        If trial[i] > limit:  Xi ← random,  trial[i]=0
 *
 * Velocity is derived from the displacement so agents look alive visually.
 */
#include "ArtificialBeeColony.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

static float frand() { return (float)rand() / (float)RAND_MAX; }
static float frank() { return frand() * 2.0f - 1.0f; }  // [-1,1]

// Convert minimisation fitness to maximisation for probabilities
static float toFit(float f) { return 1.0f / (1.0f + f); }

void ArtificialBeeColony::initialize(std::vector<Agent>& agents,
    Scenario& scenario)
{
    m_limit = std::max(10, (int)agents.size() / 2);
    m_bestFitness = 1e9f;
    m_fitness.resize(agents.size());

    for (int i = 0; i < (int)agents.size(); ++i) {
        agents[i].trial = 0;
        float f = scenario.evaluateFitness(agents[i].x, agents[i].y);
        m_fitness[i] = f;
        if (f < m_bestFitness) {
            m_bestFitness = f;
            m_bestX = agents[i].x;
            m_bestY = agents[i].y;
        }
    }
}

// Helper: try a neighbourhood move for agent i, return true if improved
static bool tryNeighbour(int i, std::vector<Agent>& agents,
    std::vector<float>& fit, Scenario& scenario)
{
    int n = (int)agents.size();
    // pick random k != i
    int k = rand() % n;
    while (k == i) k = rand() % n;

    // ABC neighbourhood: Vi[d] = Xi[d] + φ * (Xi[d] - Xk[d])
    float nx = agents[i].x + frank() * (agents[i].x - agents[k].x);
    float ny = agents[i].y + frank() * (agents[i].y - agents[k].y);

    nx = std::clamp(nx, 0.0f, 1.0f);
    ny = std::clamp(ny, 0.0f, 1.0f);

    if (!scenario.canMoveTo(agents[i].x, agents[i].y, nx, ny))
        return false;

    float fn = scenario.evaluateFitness(nx, ny);
    if (fn < fit[i]) {
        agents[i].vx = (nx - agents[i].x) * 60.0f;  // for visual motion
        agents[i].vy = (ny - agents[i].y) * 60.0f;
        agents[i].distTraveled += std::hypot(nx - agents[i].x, ny - agents[i].y);
        agents[i].x = nx;
        agents[i].y = ny;
        fit[i] = fn;
        agents[i].trial = 0;
        return true;
    }
    return false;
}

void ArtificialBeeColony::update(std::vector<Agent>& agents,
    Scenario& scenario, float dt)
{
    int n = (int)agents.size();
    if (n == 0) return;

    // ---- Phase 1: Employed bees ----
    for (int i = 0; i < n; ++i) {
        if (!agents[i].alive || agents[i].atGoal) continue;
        if (!tryNeighbour(i, agents, m_fitness, scenario))
            agents[i].trial++;
    }

    // ---- Phase 2: Calculate selection probabilities ----
    std::vector<float> prob(n);
    float sumFit = 0.0f;
    for (int i = 0; i < n; ++i)
        sumFit += toFit(m_fitness[i]);

    for (int i = 0; i < n; ++i)
        prob[i] = toFit(m_fitness[i]) / (sumFit + 1e-9f);

    // ---- Phase 3: Onlooker bees ----
    // Each onlooker selects a source with probability Pi, then explores it
    for (int bee = 0; bee < n; ++bee) {
        float r = frand();
        float cum = 0.0f;
        int chosen = 0;
        for (int i = 0; i < n; ++i) {
            cum += prob[i];
            if (r <= cum) { chosen = i; break; }
        }
        if (!agents[chosen].alive || agents[chosen].atGoal) continue;
        if (!tryNeighbour(chosen, agents, m_fitness, scenario))
            agents[chosen].trial++;
    }

    // ---- Phase 4: Scout bees – abandon exhausted sources ----
    for (int i = 0; i < n; ++i) {
        if (!agents[i].alive || agents[i].atGoal) continue;
        if (agents[i].trial > m_limit) {
            agents[i].x = frand();
            agents[i].y = frand();
            agents[i].trial = 0;
            m_fitness[i] = scenario.evaluateFitness(agents[i].x, agents[i].y);
        }
    }

    // ---- Update stats & goal check ----
    m_bestFitness = 1e9f;
    for (int i = 0; i < n; ++i) {
        agents[i].timeAlive += dt;
        if (m_fitness[i] < m_bestFitness) {
            m_bestFitness = m_fitness[i];
            m_bestX = agents[i].x;
            m_bestY = agents[i].y;
        }
        if (scenario.isAtGoal(agents[i].x, agents[i].y))
            agents[i].atGoal = true;
    }
}

void ArtificialBeeColony::drawOverlay(float xOffset, float widthScale,
    float yTop, float height)
{
    // Highlight global best food source
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float px = xOffset + m_bestX * widthScale * 800.0f;
    float py = yTop + m_bestY * height;
    dl->AddCircle(ImVec2(px, py), 10.0f, IM_COL32(255, 180, 0, 220), 6, 2.5f);
    dl->AddLine(ImVec2(px - 12, py), ImVec2(px + 12, py), IM_COL32(255, 180, 0, 180), 1.5f);
    dl->AddLine(ImVec2(px, py - 12), ImVec2(px, py + 12), IM_COL32(255, 180, 0, 180), 1.5f);
}