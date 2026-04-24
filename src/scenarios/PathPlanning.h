#pragma once
#include "Scenario.h"
#include "Agent.h"
#include <vector>

class PathPlanning : public Scenario {
public:
    PathPlanning(int width, int height);

    bool isMoveValid(float x1, float y1, float x2, float y2) const;
    bool canMove(int x, int y, int dir) const;
    const char* getName() const override;
    void reset(std::vector<Agent>& agents) override;
    void update(float deltaTime, std::vector<Agent>& agents) override;
    bool isFinished() const override;
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    void draw(const std::vector<Agent>& agents, float xOffset, float widthScale) override;

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
