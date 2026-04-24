#pragma once
#include "SwarmAlgorithm.h"
#include "../core/Agent.h"
#include "../scenarios/Scenario.h"
#include <vector>

class ParticleSwarmOptimization : public SwarmAlgorithm {
public:
    const char* getName() const override;
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
    float fitness(float x, float y, Scenario& scenario) const;

private:
    struct PersonalBest {
        float x, y;
        float fitness;
    };

    std::vector<PersonalBest> m_personalBests;
    float m_globalBestX = 0.5f;
    float m_globalBestY = 0.5f;
    float m_globalBestFitness = 1e9f;

    float fitness(float x, float y) const {
        float dx = x - 1.0f;
        float dy = y - 1.0f;
        return dx * dx + dy * dy;
    }
};