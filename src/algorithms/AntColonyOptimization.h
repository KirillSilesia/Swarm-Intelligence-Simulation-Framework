#pragma once
#include "SwarmAlgorithm.h"
#include <vector>

class AntColonyOptimization : public SwarmAlgorithm {
public:
    const char* getName() const override { return "Ant Colony Optimization"; }
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
    void drawOverlay(const Scenario& scenario) override;

private:
    static constexpr int GRID = 40;

    float m_tau[GRID][GRID];

    struct AntPath {
        std::vector<std::pair<int, int>> cells;
        float cost = 0.0f;
        bool  reachedGoal = false;
    };
    std::vector<AntPath> m_paths;

    float m_alpha = 1.0f;
    float m_beta = 2.5f;
    float m_rho = 0.05f;
    float m_Q = 0.5f;
};