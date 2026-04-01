#include "FishSchoolSearch.h"
#include <cstdlib>
#include <algorithm>

const char* FishSchoolSearch::getName() const {
    return "FishSchoolSearch";
}

void FishSchoolSearch::initialize(std::vector<Agent>& agents, Scenario&) {
    for (auto& a : agents) {
        a.vx = 0.0f;
        a.vy = 0.0f;
    }
}
void FishSchoolSearch::update(std::vector<Agent>& agents, Scenario&, float dt) {
    for (auto& a : agents) {
        a.vx = (rand() % 200 - 100) / 100.0f * 0.1f;
        a.vy = (rand() % 200 - 100) / 100.0f * 0.1f;
        a.x = std::clamp(a.x + a.vx * dt, 0.0f, 1.0f);
        a.y = std::clamp(a.y + a.vy * dt, 0.0f, 1.0f);
    }
}