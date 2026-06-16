#pragma once
#include "SwarmAlgorithm.h"
#include <vector>

class AntColonyOptimization : public SwarmAlgorithm {
public:
    const char* getName() const override { return "Ant Colony Optimization"; }
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
    void drawOverlay(float xOffset, float widthScale, float yTop, float height) override;

private:
    static constexpr int GRID = 40;

    // Pheromone grid τ[y][x]
    float m_tau[GRID][GRID];

    // Per-ant path history (grid cell indices)
    struct AntPath {
        std::vector<std::pair<int, int>> cells;
        float cost = 0.0f;
        bool  reachedGoal = false;
    };
    std::vector<AntPath> m_paths;

    // Algorithm parameters
    float m_alpha = 1.0f;   // pheromone exponent
    float m_beta = 2.5f;   // heuristic exponent
    float m_rho = 0.05f;  // evaporation rate
    float m_Q = 0.5f;   // pheromone deposit constant

    float heuristic(int gx, int gy, float goalX, float goalY) const;
    float tauAlphaBetaHeuristic(int gx, int gy, float goalX, float goalY) const;
};