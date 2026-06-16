#pragma once
#include <vector>
#include "Agent.h"

class Scenario {
public:
    virtual ~Scenario() = default;

    virtual const char* getName()  const = 0;
    virtual void reset(std::vector<Agent>& agents) = 0;
    virtual void update(float dt, std::vector<Agent>& agents) = 0;
    virtual bool isFinished() const = 0;
    virtual void draw(const std::vector<Agent>& agents,
        float xOffset, float widthScale) = 0;

    // ---- Algorithm interface ------------------------------------------------
    // Lower fitness = better position.  Minimised by all algorithms.
    virtual float evaluateFitness(float x, float y) const = 0;

    // True when the proposed move from (x1,y1)->(x2,y2) is geometrically valid
    // (no wall / boundary crossing).  Default: stay in [0,1]^2.
    virtual bool canMoveTo(float /*x1*/, float /*y1*/,
        float x2, float y2) const {
        return x2 >= 0.0f && x2 <= 1.0f &&
            y2 >= 0.0f && y2 <= 1.0f;
    }

    virtual float getGoalX()  const = 0;
    virtual float getGoalY()  const = 0;
    virtual float getStartX() const { return 0.0f; }
    virtual float getStartY() const { return 0.0f; }

    // Returns true when agent (x,y) is inside the goal zone
    virtual bool isAtGoal(float x, float y) const {
        float dx = x - getGoalX();
        float dy = y - getGoalY();
        return (dx * dx + dy * dy) < 0.003f;   // ~0.055 radius
    }

    // ---- Runtime stats (written by Simulation) ------------------------------
    float elapsedTime = 0.0f;
    int   agentsAtGoal = 0;
    int   totalAgents = 0;
};