#include "TargetSearch.h"
#include <cmath>

const char* TargetSearch::getName() const {
    return "Target Search";
}

void TargetSearch::reset(std::vector<Agent>& agents) {
    m_found = false;

    for (auto& a : agents) {
        a.x = getStartX();
        a.y = getStartY();
        a.vx = 0.0f;
        a.vy = 0.0f;
        a.alive = true;
        a.atGoal = false;
    }
}

void TargetSearch::update(float /*dt*/, std::vector<Agent>& agents) {
    for (auto& a : agents) {
        if (isAtGoal(a.x, a.y)) {
            a.atGoal = true;
            m_found = true;
        }
    }
}

bool TargetSearch::isFinished() const {
    return m_found;
}

void TargetSearch::draw(const std::vector<Agent>& /*agents*/,
    float /*xOffset*/, float /*ws*/) {
    // ничего не рисуем (placeholder)
}

float TargetSearch::evaluateFitness(float x, float y) const {
    return std::hypot(x - getGoalX(), y - getGoalY());
}

float TargetSearch::getGoalX() const {
    return 1.0f;
}

float TargetSearch::getGoalY() const {
    return 1.0f;
}