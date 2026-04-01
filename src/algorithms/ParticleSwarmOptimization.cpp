#include "ParticleSwarmOptimization.h"
#include <cstdlib>
#include <algorithm>

const char* ParticleSwarmOptimization::getName() const {
    return "Particle Swarm Optimization";
}

void ParticleSwarmOptimization::initialize(std::vector<Agent>& agents, Scenario&) {
    for (auto& a : agents) {
        a.vx = 0.0f;
        a.vy = 0.0f;
    }
}

void ParticleSwarmOptimization::update(std::vector<Agent>& agents, Scenario&, float dt) {
    for (auto& a : agents) {
        a.x += a.vx * dt;
        a.y += a.vy * dt;
        // Simple random nudge for now
        a.vx = (rand() % 200 - 100) / 100.0f * 0.1f;
        a.vy = (rand() % 200 - 100) / 100.0f * 0.1f;
        a.x = std::clamp(a.x, 0.0f, 1.0f);
        a.y = std::clamp(a.y, 0.0f, 1.0f);
    }
}