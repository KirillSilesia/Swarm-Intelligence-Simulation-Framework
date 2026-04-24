#include "ParticleSwarmOptimization.h"
#include "../scenarios/PathPlanning.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

static float frand() { return (float)rand() / RAND_MAX; }

const char* ParticleSwarmOptimization::getName() const {
    return "Particle Swarm Optimization";
}

float ParticleSwarmOptimization::fitness(float x, float y, Scenario& scenario) const {
    float dx = x - 1.0f;
    float dy = y - 1.0f;
    float dist = dx * dx + dy * dy;

    PathPlanning* maze = (PathPlanning*)&scenario;

    float penalty = 0.0f;

    if (maze) {
        const float eps = 0.01f;
        int blocked = 0;

        if (!maze->isMoveValid(x, y, x + eps, y)) blocked++;
        if (!maze->isMoveValid(x, y, x - eps, y)) blocked++;
        if (!maze->isMoveValid(x, y, x, y + eps)) blocked++;
        if (!maze->isMoveValid(x, y, x, y - eps)) blocked++;

        penalty += blocked * 0.5f;
    }

    return dist + penalty;
}

void ParticleSwarmOptimization::initialize(std::vector<Agent>& agents, Scenario& scenario) {
    float range = 1.0f;

    m_personalBests.resize(agents.size());
    m_globalBestFitness = 1e9f;

    for (int i = 0; i < (int)agents.size(); ++i) {
        auto& a = agents[i];

        a.vx = -range + frand() * 2.0f * range;
        a.vy = -range + frand() * 2.0f * range;

        float f = fitness(a.x, a.y, scenario);
        m_personalBests[i] = { a.x, a.y, f };

        if (f < m_globalBestFitness) {
            m_globalBestFitness = f;
            m_globalBestX = a.x;
            m_globalBestY = a.y;
        }
    }
}

void ParticleSwarmOptimization::update(std::vector<Agent>& agents, Scenario& scenario, float dt) {
    const float w = 0.5f;
    const float phi_p = 1.4f;
    const float phi_g = 0.6f;
    const float maxV = 0.4f;

    PathPlanning* maze = dynamic_cast<PathPlanning*>(&scenario);

    for (int i = 0; i < (int)agents.size(); ++i) {
        auto& a = agents[i];
        auto& pb = m_personalBests[i];

        float rp = frand();
        float rg = frand();

        float useGlobal = (frand() < 0.7f) ? 1.0f : 0.0f;

        a.vx = w * a.vx
            + phi_p * rp * (pb.x - a.x)
            + useGlobal * phi_g * rg * (m_globalBestX - a.x);

        a.vy = w * a.vy
            + phi_p * rp * (pb.y - a.y)
            + useGlobal * phi_g * rg * (m_globalBestY - a.y);

        a.vx += (frand() - 0.5f) * 0.15f;
        a.vy += (frand() - 0.5f) * 0.15f;

        a.vx = std::clamp(a.vx, -maxV, maxV);
        a.vy = std::clamp(a.vy, -maxV, maxV);

        if (fabs(a.vx) < 0.0001f && fabs(a.vy) < 0.0001f) {
            a.vx += (frand() - 0.5f) * 0.2f;
            a.vy += (frand() - 0.5f) * 0.2f;
        }

        float newX = a.x + a.vx * dt;
        float newY = a.y + a.vy * dt;

        bool moved = false;

        if (maze && maze->isMoveValid(a.x, a.y, newX, newY)) {
            a.x = std::clamp(newX, 0.0f, 1.0f);
            a.y = std::clamp(newY, 0.0f, 1.0f);
            moved = true;
        }

        if (!moved) {
            if (maze && maze->isMoveValid(a.x, a.y, a.x + a.vx * dt, a.y)) {
                a.x = std::clamp(a.x + a.vx * dt, 0.0f, 1.0f);
                a.vy *= -0.2f;
                moved = true;
            }
            else if (maze && maze->isMoveValid(a.x, a.y, a.x, a.y + a.vy * dt)) {
                a.y = std::clamp(a.y + a.vy * dt, 0.0f, 1.0f);
                a.vx *= -0.2f;
                moved = true;
            }
        }

        if (!moved) {
            a.vx = (frand() - 0.5f) * maxV;
            a.vy = (frand() - 0.5f) * maxV;

            float tryX = a.x + a.vx * dt;
            float tryY = a.y + a.vy * dt;

            if (!maze || maze->isMoveValid(a.x, a.y, tryX, tryY)) {
                a.x = std::clamp(tryX, 0.0f, 1.0f);
                a.y = std::clamp(tryY, 0.0f, 1.0f);
            }

            pb.fitness += 0.3f;
        }

        float f = fitness(a.x, a.y, scenario);

        if (f < pb.fitness) {
            pb = { a.x, a.y, f };

            if (f < m_globalBestFitness) {
                m_globalBestFitness = f;
                m_globalBestX = a.x;
                m_globalBestY = a.y;
            }
        }
    }
}