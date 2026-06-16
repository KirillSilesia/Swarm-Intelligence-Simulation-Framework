#pragma once
#include "Scenario.h"
#include <vector>

class PathPlanning : public Scenario {
public:
    PathPlanning(int width, int height);

    const char* getName() const override;
    void reset(std::vector<Agent>& agents) override;
    void update(float dt, std::vector<Agent>& agents) override;
    bool isFinished() const override;
    void draw(const std::vector<Agent>& agents, float xOffset, float widthScale) override;

    // Scenario interface
    float evaluateFitness(float x, float y) const override;
    bool  canMoveTo(float x1, float y1, float x2, float y2) const override;
    float getGoalX()  const override { return (m_width - 1 + 0.5f) / m_width; }
    float getGoalY()  const override { return (m_height - 1 + 0.5f) / m_height; }
    float getStartX() const override { return 0.5f / m_width; }
    float getStartY() const override { return 0.5f / m_height; }

    int getWidth()  const { return m_width; }
    int getHeight() const { return m_height; }

private:
    struct Cell {
        bool visited = false;
        bool walls[4] = { true, true, true, true }; // N E S W
    };
    int m_width, m_height;
    bool m_finished = false;
    std::vector<Cell> m_cells;

    int  index(int x, int y) const { return x + y * m_width; }
    void generateMaze();
};