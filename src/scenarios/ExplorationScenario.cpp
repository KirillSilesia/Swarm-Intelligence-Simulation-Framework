#include "ExplorationScenario.h"
#include <cstdlib>

const char* ExplorationScenario::getName() const {
    return "Exploration Scenario";
}

void ExplorationScenario::reset() {
    finished = false;
}

void ExplorationScenario::update(float) {
    if (rand() % 500 == 0)
        finished = true;
}

bool ExplorationScenario::isFinished() const {
    return finished;
}
