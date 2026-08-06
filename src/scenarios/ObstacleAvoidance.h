#pragma once
#include "Scenario.h"
#include <vector>

struct MovingObstacle {
    float rx, ry;
    float vx, vy;
    float normSize;
    float displaySize;
    int   shape;
    float rotation;
    float rotSpeed;
};

class ObstacleAvoidance : public Scenario {
public:
    ObstacleAvoidance(int width, int height);

    const char* getName() const override;
    void reset(std::vector<Agent>& agents) override;
    void update(float dt, std::vector<Agent>& agents) override;
    bool isFinished() const override;
    void draw(const std::vector<Agent>& agents, float xOffset, float widthScale) override;

    float evaluateFitness(float x, float y) const override;
    float getGoalX()  const override { return 0.97f; }
    float getGoalY()  const override { return 0.50f; }
    float getStartX() const override { return 0.03f; }
    float getStartY() const override { return 0.50f; }
    bool  canMoveTo(float x1, float y1, float x2, float y2) const override;
    bool  guidanceDir(float x, float y, float& dirX, float& dirY) const override;

private:
    int   m_width, m_height;
    bool  m_finished = false;
    float m_corridorW = 600.0f;
    float m_corridorH = 300.0f;
    float m_gapHalf = 35.0f;

    std::vector<MovingObstacle> m_obstacles;
    int m_collisions = 0;

    void generateObstacles();
};