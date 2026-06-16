#pragma once
#include "SwarmAlgorithm.h"
#include <vector>

class FishSchoolSearch : public SwarmAlgorithm {
public:
    const char* getName() const override { return "Fish School Search"; }
    void initialize(std::vector<Agent>& agents, Scenario& scenario) override;
    void update(std::vector<Agent>& agents, Scenario& scenario, float dt) override;
    void drawOverlay(float xOffset, float widthScale, float yTop, float height) override;

private:
    float m_stepInd = 0.12f;   // individual step size
    float m_stepCol = 0.04f;   // collective step size
    float m_prevSchoolWeight = 0.0f;
    float m_baryX = 0.5f, m_baryY = 0.5f;
};