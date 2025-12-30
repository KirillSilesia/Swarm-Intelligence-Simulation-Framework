#include "MazeScenario.h"

MazeScenario::MazeScenario(int width, int height)
    : m_width(width), m_height(height), m_finished(false) {}

const char* MazeScenario::getName() const {
    return "Maze";
}

void MazeScenario::reset() {
    m_finished = false;
    // reset
}

void MazeScenario::update(float deltaTime) {
    //update
}

bool MazeScenario::isFinished() const {
    return m_finished;
}
