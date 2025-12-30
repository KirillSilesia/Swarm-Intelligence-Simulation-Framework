#pragma once
#include "Scenario.h"

class MazeScenario : public Scenario {
public:
    MazeScenario(int width, int height);

    const char* getName() const override;
    void reset() override;
    void update(float deltaTime) override;
    bool isFinished() const override;

private:
    int m_width;
    int m_height;
    bool m_finished;
};
