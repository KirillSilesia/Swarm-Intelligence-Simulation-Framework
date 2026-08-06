#pragma once
#include "SwarmAlgorithm.h"
#include <cmath>
#include <vector>

class ArtificialBeeColony : public SwarmAlgorithm {
public:
    const char* getName() const override { return "Artificial Bee Colony"; }
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
    void drawOverlay(const Scenario& scenario) override;

private:
    std::vector<float> m_fitness;
    int   m_limit = 20;
    float m_bestX = 0.5f, m_bestY = 0.5f;
    float m_bestFitness = 1e9f;
};