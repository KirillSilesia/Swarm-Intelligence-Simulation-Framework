#include "AntColonyOptimization.h"
#include <cstdlib>

const char* AntColonyOptimization::getName() const {
    return "Ant Colony Optimization";
}

void AntColonyOptimization::initialize(
    std::vector<Agent>& agents,
    Scenario&
) {
    for (auto& a : agents) {
        a.x = rand() % 100;
        a.y = rand() % 100;
        a.vx = a.vy = 0.0f;
    }
}

void AntColonyOptimization::update(
    std::vector<Agent>& agents,
    Scenario&,
    float deltaTime
) {
    for (auto& a : agents) {
        a.x += (rand() % 3 - 1) * deltaTime * 50.0f;
        a.y += (rand() % 3 - 1) * deltaTime * 50.0f;
    }
}