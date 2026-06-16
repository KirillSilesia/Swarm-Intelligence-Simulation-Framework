#pragma once
#include "SwarmAlgorithm.h"
#include <vector>

class ParticleSwarmOptimization : public SwarmAlgorithm {
public:
    const char* getName() const override { return "Particle Swarm Optimization"; }
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
    void drawOverlay(float xOffset, float widthScale, float yTop, float height) override;

private:
    float m_globalBestX = 0.5f;
    float m_globalBestY = 0.5f;
    float m_globalBestFitness = 1e9f;
};