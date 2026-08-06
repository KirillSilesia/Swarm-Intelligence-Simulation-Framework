#pragma once
#include "SwarmAlgorithm.h"
#include <vector>

class FishSchoolSearch : public SwarmAlgorithm {
public:
    const char* getName() const override { return "Fish School Search"; }
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
    void drawOverlay(const Scenario& scenario) override;

private:
    float m_stepInd = 0.12f;
    float m_stepCol = 0.04f;
    float m_prevSchoolWeight = 0.0f;
    float m_baryX = 0.5f, m_baryY = 0.5f;
};