#include "FishSchoolSearch.h"
#include <cstdlib>

const char* FishSchoolSearch::getName() const {
    return "FishSchoolSearch";
}

void FishSchoolSearch::initialize(std::vector<Agent>& agents, Scenario&) {
    for (auto& a : agents) {
        a.x = rand() % 100;
        a.y = rand() % 100;
    }
}

void FishSchoolSearch::update(std::vector<Agent>& agents, Scenario&, float dt) {
    for (auto& a : agents) {
        a.x += (rand() % 3 - 1) * dt * 40;
        a.y += (rand() % 3 - 1) * dt * 40;
    }
}
