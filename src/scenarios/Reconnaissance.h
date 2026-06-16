#pragma once
#include "Scenario.h"
#include "imgui.h"
#include <vector>

struct ReconObject {
    float x, y;        // normalized [0,1]
    float radius;      // display radius (pixels)
    int   priority;    // 3=high, 2=medium, 1=low
    bool  found = false;
    float foundTime = -1.0f;
};

class Reconnaissance : public Scenario {
public:
    explicit Reconnaissance(ImVec2 fieldSize);

    const char* getName() const override;
    void reset(std::vector<Agent>& agents) override;
    void update(float dt, std::vector<Agent>& agents) override;
    bool isFinished() const override;
    void draw(const std::vector<Agent>& agents, float xOffset, float widthScale) override;

    // Scenario interface
    float evaluateFitness(float x, float y) const override;
    float getGoalX()  const override { return 0.95f; }
    float getGoalY()  const override { return 0.95f; }
    float getStartX() const override { return 0.05f; }
    float getStartY() const override { return 0.05f; }

    // Used by reports
    int highFound() const;
    int medFound()  const;
    int lowFound()  const;

private:
    ImVec2 m_fieldSize;
    std::vector<ReconObject> m_objects;
    bool   m_finished = false;
};