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

    virtual float evaluateFitness(float x, float y) const = 0;

    virtual bool canMoveTo(float /*x1*/, float /*y1*/,
        float x2, float y2) const {
        return x2 >= 0.0f && x2 <= 1.0f &&
            y2 >= 0.0f && y2 <= 1.0f;
    }

    // Local "which way is downhill toward the goal" hint at (x,y), written as a
    // unit vector to (dirX,dirY). Unlike the straight-line pull toward a distant
    // global best, this follows the actual open corridor, so agents navigate a
    // maze instead of jamming against the walls between them and the goal.
    // Returns false when the scenario offers no guidance (open-field scenarios),
    // leaving the algorithm's own dynamics untouched.
    virtual bool guidanceDir(float /*x*/, float /*y*/,
        float& dirX, float& dirY) const {
        dirX = dirY = 0.0f;
        return false;
    }

    virtual float getGoalX()  const = 0;
    virtual float getGoalY()  const = 0;
    virtual float getStartX() const { return 0.0f; }
    virtual float getStartY() const { return 0.0f; }

    virtual bool isAtGoal(float x, float y) const {
        float dx = x - getGoalX();
        float dy = y - getGoalY();
        return (dx * dx + dy * dy) < 0.003f;
    }

    float elapsedTime = 0.0f;
    int   agentsAtGoal = 0;
    int   totalAgents = 0;

    // On-screen rectangle that the normalized [0,1]x[0,1] play area is drawn
    // into. Each scenario sets this at the top of draw(); algorithm overlays
    // read it so their visuals stay aligned with whatever the scenario drew.
    float viewX = 0.0f, viewY = 0.0f, viewW = 1.0f, viewH = 1.0f;
};