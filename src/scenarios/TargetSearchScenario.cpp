#include "TargetSearchScenario.h"
#include <cstdlib> 

const char* TargetSearchScenario::getName() const {
    return "Target Search";
}

void TargetSearchScenario::reset() {
    found = false;
}

void TargetSearchScenario::update(float) {
    if (rand() % 500 == 0)
        found = true;
}

bool TargetSearchScenario::isFinished() const {
    return found;
}
