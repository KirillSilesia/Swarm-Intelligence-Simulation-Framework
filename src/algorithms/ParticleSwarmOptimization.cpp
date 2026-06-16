/*
 * Particle Swarm Optimization – exact spec implementation.
 *
 * for each particle i:
 *   xi ~ U(blo,bup)   pi <- xi   vi ~ U(-|bup-blo|,|bup-blo|)
 *   if f(pi)<f(g): g <- pi
 *
 * per update:
 *   for each dim d:
 *     rp,rg ~ U(0,1)
 *     vi,d <- w*vi,d + φp*rp*(pi,d-xi,d) + φg*rg*(gd-xi,d)
 *   xi <- xi + vi
 *   if f(xi)<f(pi): pi <- xi;  if f(pi)<f(g): g <- pi
 */
#include "ParticleSwarmOptimization.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

static float frand() { return (float)rand() / (float)RAND_MAX; }

void ParticleSwarmOptimization::initialize(std::vector<Agent>& agents,
    Scenario& scenario)
{
    m_globalBestFitness = 1e9f;

    float range = 1.0f;   // bup - blo

    for (auto& a : agents) {
        // Position already set by scenario::reset()
        a.vx = -range + frand() * 2.0f * range;
        a.vy = -range + frand() * 2.0f * range;

        float f = scenario.evaluateFitness(a.x, a.y);
        a.bestX = a.x;
        a.bestY = a.y;
        a.bestFitness = f;

        if (f < m_globalBestFitness) {
            m_globalBestFitness = f;
            m_globalBestX = a.x;
            m_globalBestY = a.y;
        }
    }
}

void ParticleSwarmOptimization::update(std::vector<Agent>& agents,
    Scenario& scenario, float dt)
{
    const float w = 0.729f;   // inertia weight
    const float phiP = 1.494f;  // cognitive coefficient
    const float phiG = 1.494f;  // social coefficient
    const float maxV = 0.35f;

    for (auto& a : agents) {
        if (!a.alive || a.atGoal) continue;

        // -- velocity update (per spec, no random flags or noise) --
        float rp = frand(), rg = frand();

        a.vx = w * a.vx
            + phiP * rp * (a.bestX - a.x)
            + phiG * rg * (m_globalBestX - a.x);

        a.vy = w * a.vy
            + phiP * rp * (a.bestY - a.y)
            + phiG * rg * (m_globalBestY - a.y);

        a.vx = std::clamp(a.vx, -maxV, maxV);
        a.vy = std::clamp(a.vy, -maxV, maxV);

        // -- position update --
        float nx = a.x + a.vx * dt;
        float ny = a.y + a.vy * dt;

        if (scenario.canMoveTo(a.x, a.y, nx, ny)) {
            float ox = a.x, oy = a.y;
            a.x = std::clamp(nx, 0.0f, 1.0f);
            a.y = std::clamp(ny, 0.0f, 1.0f);
            float moved = std::hypot(a.x - ox, a.y - oy);
            a.distTraveled += moved;
        }
        else {
            // Reflect velocity on blocked axis
            float tx = a.x + a.vx * dt, ty = a.y;
            float tx2 = a.x, ty2 = a.y + a.vy * dt;
            if (scenario.canMoveTo(a.x, a.y, tx, ty)) {
                a.x = std::clamp(tx, 0.0f, 1.0f);
                a.vy *= -0.5f;
            }
            else if (scenario.canMoveTo(a.x, a.y, tx2, ty2)) {
                a.y = std::clamp(ty2, 0.0f, 1.0f);
                a.vx *= -0.5f;
            }
            else {
                // Fully stuck – random kick to escape corner
                a.vx = (frand() - 0.5f) * maxV;
                a.vy = (frand() - 0.5f) * maxV;
            }
        }

        // -- personal / global best update --
        float f = scenario.evaluateFitness(a.x, a.y);
        if (f < a.bestFitness) {
            a.bestFitness = f;
            a.bestX = a.x;
            a.bestY = a.y;
            if (f < m_globalBestFitness) {
                m_globalBestFitness = f;
                m_globalBestX = a.x;
                m_globalBestY = a.y;
            }
        }

        a.timeAlive += dt;

        if (scenario.isAtGoal(a.x, a.y))
            a.atGoal = true;
    }
}

void ParticleSwarmOptimization::drawOverlay(float xOffset, float widthScale,
    float yTop, float height)
{
    // Draw global best as a bright star
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float px = xOffset + m_globalBestX * widthScale * 800.0f;  // approximate
    float py = yTop + m_globalBestY * height;
    dl->AddCircle(ImVec2(px, py), 8.0f, IM_COL32(255, 220, 0, 200), 12, 2.0f);
}