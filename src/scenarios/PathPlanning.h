#pragma once
#include "Scenario.h"
#include <vector>

class PathPlanning : public Scenario {
public:
    PathPlanning(int width, int height);

    const char* getName() const override;
    void reset() override;
    void update(float deltaTime) override;
    bool isFinished() const override;
    void draw() override;

private:
    struct Cell {
        bool visited = false;
        bool walls[4] = { true, true, true, true };
    };

    int m_width;
    int m_height;
    bool m_finished;

    std::vector<Cell> m_cells;

    int index(int x, int y) const;
    void generateMaze();
};
