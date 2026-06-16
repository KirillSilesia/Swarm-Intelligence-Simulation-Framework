/*
 * Fish School Search – per spec.
 *
 *  1. Individual movement (exploration):
 *       candidate = position[i] + stepInd * rand_direction
 *       If fitness(candidate) < fitness(current): accept move
 *
 *  2. Feeding (weight update):
 *       weight[i] += ΔF[i] / max_ΔF
 *       (ΔF = fitness improvement this step, 0 if none)
 *
 *  3. Collective-instinctive movement:
 *       displacement = Σ(pos_i * ΔF_i) / Σ(ΔF_i)
 *       All fish move: pos[i] += stepCol * displacement
 *
 *  4. Collective-volitive movement:
 *       barycenter = Σ(weight_i * pos_i) / Σ(weight_i)
 *       If total_weight increased: move toward barycenter
 *       Else:                      move away from barycenter
 *
 *  5. Decay step sizes gradually.
 */
#include "FishSchoolSearch.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

static float frand() { return (float)rand() / (float)RAND_MAX; }

void FishSchoolSearch::initialize(std::vector<Agent>& agents,
    Scenario& scenario)
{
    m_stepInd = 0.12f;
    m_stepCol = 0.04f;
    m_prevSchoolWeight = 0.0f;

    for (auto& a : agents) {
        a.weight = 1.0f;
        a.prevFitness = scenario.evaluateFitness(a.x, a.y);
        a.fitnessGain = 0.0f;
        a.indivDx = a.indivDy = 0.0f;
    }

    float tw = 0.0f;
    for (const auto& a : agents) tw += a.weight;
    m_prevSchoolWeight = tw;
}

void FishSchoolSearch::update(std::vector<Agent>& agents,
    Scenario& scenario, float dt)
{
    int n = (int)agents.size();
    if (n == 0) return;

    // ---- 1. Individual movement ----
    float maxDeltaF = 0.0f;

    for (auto& a : agents) {
        if (!a.alive || a.atGoal) { a.fitnessGain = 0.0f; continue; }

        // Random unit direction
        float angle = frand() * 6.2831853f;
        float dx = std::cos(angle) * m_stepInd;
        float dy = std::sin(angle) * m_stepInd;

        float nx = std::clamp(a.x + dx, 0.0f, 1.0f);
        float ny = std::clamp(a.y + dy, 0.0f, 1.0f);

        if (!scenario.canMoveTo(a.x, a.y, nx, ny)) {
            a.fitnessGain = 0.0f; a.indivDx = a.indivDy = 0.0f;
            continue;
        }

        float fnew = scenario.evaluateFitness(nx, ny);
        float dF = a.prevFitness - fnew;   // positive = improvement

        if (dF > 0.0f) {
            a.indivDx = nx - a.x;
            a.indivDy = ny - a.y;
            a.distTraveled += std::hypot(a.indivDx, a.indivDy);
            a.x = nx; a.y = ny;
            a.fitnessGain = dF;
            if (dF > maxDeltaF) maxDeltaF = dF;
        }
        else {
            a.fitnessGain = 0.0f;
            a.indivDx = a.indivDy = 0.0f;
        }
        a.prevFitness = scenario.evaluateFitness(a.x, a.y);
    }

    // ---- 2. Feeding ----
    if (maxDeltaF > 1e-9f) {
        for (auto& a : agents) {
            if (!a.alive || a.atGoal) continue;
            a.weight += a.fitnessGain / maxDeltaF;
            a.weight = std::clamp(a.weight, 0.01f, 10.0f);
        }
    }

    // ---- 3. Collective-instinctive movement ----
    float sumDF = 0.0f, collectX = 0.0f, collectY = 0.0f;
    for (const auto& a : agents) {
        if (!a.alive || a.atGoal) continue;
        sumDF += a.fitnessGain;
        collectX += a.indivDx * a.fitnessGain;
        collectY += a.indivDy * a.fitnessGain;
    }

    if (sumDF > 1e-9f) {
        float instX = (collectX / sumDF) * m_stepCol;
        float instY = (collectY / sumDF) * m_stepCol;

        for (auto& a : agents) {
            if (!a.alive || a.atGoal) continue;
            float nx = std::clamp(a.x + instX, 0.0f, 1.0f);
            float ny = std::clamp(a.y + instY, 0.0f, 1.0f);
            if (scenario.canMoveTo(a.x, a.y, nx, ny)) {
                a.distTraveled += std::hypot(nx - a.x, ny - a.y);
                a.x = nx; a.y = ny;
            }
        }
    }

    // ---- 4. Collective-volitive movement ----
    float totalWeight = 0.0f, bx = 0.0f, by = 0.0f;
    for (const auto& a : agents) {
        if (!a.alive || a.atGoal) continue;
        totalWeight += a.weight;
        bx += a.x * a.weight;
        by += a.y * a.weight;
    }

    if (totalWeight > 1e-9f) {
        bx /= totalWeight; by /= totalWeight;
        m_baryX = bx; m_baryY = by;

        bool converge = (totalWeight >= m_prevSchoolWeight);
        for (auto& a : agents) {
            if (!a.alive || a.atGoal) continue;
            float ddx = bx - a.x;
            float ddy = by - a.y;
            float dist = std::hypot(ddx, ddy) + 1e-9f;
            float step = m_stepCol * (converge ? 1.0f : -1.0f);
            float nx = std::clamp(a.x + step * ddx / dist, 0.0f, 1.0f);
            float ny = std::clamp(a.y + step * ddy / dist, 0.0f, 1.0f);
            if (scenario.canMoveTo(a.x, a.y, nx, ny)) {
                a.distTraveled += std::hypot(nx - a.x, ny - a.y);
                a.x = nx; a.y = ny;
            }
        }
    }
    m_prevSchoolWeight = totalWeight;

    // ---- 5. Decay step sizes ----
    m_stepInd = std::max(m_stepInd * 0.9995f, 0.005f);
    m_stepCol = std::max(m_stepCol * 0.9995f, 0.001f);

    // ---- Stats & goal check ----
    for (auto& a : agents) {
        a.timeAlive += dt;
        a.vx = a.indivDx / (dt + 1e-6f);
        a.vy = a.indivDy / (dt + 1e-6f);
        if (scenario.isAtGoal(a.x, a.y))
            a.atGoal = true;
    }
}

void FishSchoolSearch::drawOverlay(float xOffset, float widthScale,
    float yTop, float height)
{
    // Draw barycenter of school
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float px = xOffset + m_baryX * widthScale * 800.0f;
    float py = yTop + m_baryY * height;
    dl->AddCircle(ImVec2(px, py), 14.0f, IM_COL32(100, 200, 255, 160), 8, 2.0f);
    dl->AddText(ImVec2(px + 16, py - 7), IM_COL32(100, 200, 255, 220), "B");
}