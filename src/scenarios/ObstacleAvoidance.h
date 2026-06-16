#pragma once
#include "Scenario.h"
#include <vector>

struct MovingObstacle {
    float rx, ry;       // relative position [0,1] inside corridor
    float vx, vy;       // velocity (pixels/s → converted to relative)
    float normSize;     // collision/display radius in normalized coords
    float displaySize;  // radius in pixels for drawing
    int   shape;        // 0=circle, 1=rect, 2=triangle
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

    // Scenario interface
    float evaluateFitness(float x, float y) const override;
    float getGoalX()  const override { return 0.97f; }
    float getGoalY()  const override { return 0.50f; }
    float getStartX() const override { return 0.03f; }
    float getStartY() const override { return 0.50f; }
    bool  canMoveTo(float x1, float y1, float x2, float y2) const override;

private:
    int   m_width, m_height;
    bool  m_finished = false;
    float m_corridorW = 600.0f;
    float m_corridorH = 300.0f;
    float m_gapHalf = 35.0f;   // half-height of entry/exit gap

    std::vector<MovingObstacle> m_obstacles;
    int m_collisions = 0;

    void generateObstacles();
};