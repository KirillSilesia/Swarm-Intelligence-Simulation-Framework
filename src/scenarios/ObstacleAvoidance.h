#pragma once
#include "Scenario.h"
#include <vector>

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
};

enum class ObstacleShape {
    Circle,
    Rectangle,
    Triangle
};

struct Obstacle {
    Vec2 position;
    Vec2 velocity;
    float size;
    ObstacleShape shape;
    float rotation;
    float rotationSpeed;
};

class ObstacleAvoidance : public Scenario {
public:
    ObstacleAvoidance(int width, int height);
    const char* getName() const override;
    void reset() override;
    void update(float deltaTime) override;
    bool isFinished() const override;
    void draw() override;

private:
    int m_width;
    int m_height;
    bool m_finished;

    Vec2 m_corridorTopLeft;
    Vec2 m_corridorBottomRight;
    float m_corridorWidth;
    float m_corridorHeight;
    float m_entranceY;
    float m_exitY;
    float m_gapSize;

    std::vector<Obstacle> m_obstacles;

    void generateObstacles();
    bool isInsideCorridor(const Vec2& pos) const;
};