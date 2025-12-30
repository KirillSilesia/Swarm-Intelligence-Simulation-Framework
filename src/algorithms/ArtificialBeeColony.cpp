#include "ArtificialBeeColony.h"
#include <cstdlib>

const char* ArtificialBeeColony::getName() const {
    return "Artificial Bee Colony";
}

void ArtificialBeeColony::initialize(std::vector<Agent>& agents, Scenario&) {
    for (auto& a : agents) {
        a.x = rand() % 100;
        a.y = rand() % 100;
    }
}

void ArtificialBeeColony::update(std::vector<Agent>& agents, Scenario&, float dt) {
    for (auto& a : agents) {
        a.x += (rand() % 3 - 1) * dt * 40;
        a.y += (rand() % 3 - 1) * dt * 40;
    }
}
