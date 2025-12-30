#include "ParticleSwarmOptimization.h"
#include <cstdlib>

const char* ParticleSwarmOptimization::getName() const {
    return "Particle Swarm Optimization";
}

void ParticleSwarmOptimization::initialize(std::vector<Agent>& agents, Scenario&) {
    for (auto& a : agents) {
        a.x = rand() % 100;
        a.y = rand() % 100;
    }
}

void ParticleSwarmOptimization::update(std::vector<Agent>& agents, Scenario&, float dt) {
    for (auto& a : agents) {
        a.x += (rand() % 3 - 1) * dt * 40;
        a.y += (rand() % 3 - 1) * dt * 40;
    }
}
