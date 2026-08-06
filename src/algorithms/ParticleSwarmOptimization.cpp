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

    float range = 1.0f;

    for (auto& a : agents) {
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
    const float w = 0.729f;
    const float phiP = 1.494f;
    const float phiG = 1.494f;
    const float maxV = 0.35f;
    const float guideStrength = 1.5f;  // corridor-following pull in mazes
    const int   stuckThreshold = 5;
    const int   stagnationLimit = 20;

    // The fitness landscape can shift during a run -- reconnaissance targets get
    // found, obstacles drift -- which leaves the stored personal and global
    // bests holding stale scores for a world that no longer exists. The swarm
    // then keeps chasing a vanished optimum and freezes in a clump. Re-score the
    // remembered best positions against the current landscape so the swarm lets
    // go of found targets and flows on to what is still out there.
    m_globalBestFitness = scenario.evaluateFitness(m_globalBestX, m_globalBestY);

    float fitnessBeforeTick = m_globalBestFitness;

    // Canonical PSO has a known failure mode: once every particle's personal
    // best stops improving, (bestX - x) and (globalBestX - x) both collapse
    // to ~0 simultaneously, velocity decays to exactly zero, and the swarm
    // freezes at whatever point it happened to converge to -- regardless of
    // how far that is from the goal. The fix is  re-energize velocity
    // whenever the swarm has gone stagnant, so particles keep exploring
    // instead of sitting at a fixed point with zero residual motion.
    bool reinject = m_stagnantTicks >= stagnationLimit;

    for (auto& a : agents) {
        if (!a.alive || a.atGoal) continue;

        // Refresh this particle's remembered-best score under the current
        // landscape (see above) before it steers toward that position.
        a.bestFitness = scenario.evaluateFitness(a.bestX, a.bestY);

        float escapeFactor = (a.trial > stuckThreshold)
            ? 1.0f / (1.0f + 0.3f * (a.trial - stuckThreshold))
            : 1.0f;

        float rp = frand(), rg = frand();

        a.vx = w * a.vx
            + phiP * rp * (a.bestX - a.x)
            + phiG * rg * escapeFactor * (m_globalBestX - a.x);

        a.vy = w * a.vy
            + phiP * rp * (a.bestY - a.y)
            + phiG * rg * escapeFactor * (m_globalBestY - a.y);

        // Steer along the maze corridor. This dominates the straight-line pull
        // toward the (possibly wall-blocked) global best, so particles round
        // corners instead of grinding into the wall that faces the goal.
        float gdx, gdy;
        if (scenario.guidanceDir(a.x, a.y, gdx, gdy)) {
            a.vx += guideStrength * gdx;
            a.vy += guideStrength * gdy;
        }

        if (reinject) {
            a.vx += (frand() - 0.5f) * maxV;
            a.vy += (frand() - 0.5f) * maxV;
        }

        a.vx = std::clamp(a.vx, -maxV, maxV);
        a.vy = std::clamp(a.vy, -maxV, maxV);

        float nx = a.x + a.vx * dt;
        float ny = a.y + a.vy * dt;

        if (scenario.canMoveTo(a.x, a.y, nx, ny)) {
            float ox = a.x, oy = a.y;
            a.x = std::clamp(nx, 0.0f, 1.0f);
            a.y = std::clamp(ny, 0.0f, 1.0f);
            float moved = std::hypot(a.x - ox, a.y - oy);
            a.distTraveled += moved;
            a.trial = 0;
        }
        else {
            float tx = a.x + a.vx * dt, ty = a.y;
            float tx2 = a.x, ty2 = a.y + a.vy * dt;
            if (scenario.canMoveTo(a.x, a.y, tx, ty)) {
                a.x = std::clamp(tx, 0.0f, 1.0f);
                a.vy *= -0.5f;
                a.trial = 0;
            }
            else if (scenario.canMoveTo(a.x, a.y, tx2, ty2)) {
                a.y = std::clamp(ty2, 0.0f, 1.0f);
                a.vx *= -0.5f;
                a.trial = 0;
            }
            else {
                ++a.trial;
                float speed = std::hypot(a.vx, a.vy) + 1e-6f;
                float perpX = -a.vy / speed;
                float perpY = a.vx / speed;
                float sign = (a.trial % 2 == 0) ? 1.0f : -1.0f;
                float boldness = std::min(1.0f, 0.15f * a.trial);

                a.vx = sign * perpX * maxV * (0.4f + 0.6f * boldness)
                    + (frand() - 0.5f) * maxV * boldness;
                a.vy = sign * perpY * maxV * (0.4f + 0.6f * boldness)
                    + (frand() - 0.5f) * maxV * boldness;
            }
        }

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

    if (m_globalBestFitness < fitnessBeforeTick - 1e-6f)
        m_stagnantTicks = 0;
    else
        ++m_stagnantTicks;
}

void ParticleSwarmOptimization::drawOverlay(const Scenario& scenario)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float px = scenario.viewX + m_globalBestX * scenario.viewW;
    float py = scenario.viewY + m_globalBestY * scenario.viewH;
    dl->AddCircle(ImVec2(px, py), 8.0f, IM_COL32(255, 220, 0, 200), 12, 2.0f);
}