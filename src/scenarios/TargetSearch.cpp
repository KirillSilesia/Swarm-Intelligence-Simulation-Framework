#include "TargetSearch.h"
#include <cstdlib> 

const char* TargetSearch::getName() const {
    return "Target Search";
}

void TargetSearch::reset() {
    found = false;
}

void TargetSearch::update(float) {
    if (rand() % 500 == 0)
        found = true;
}

bool TargetSearch::isFinished() const {
    return found;
}

void TargetSearch::draw() {
	// draw
}