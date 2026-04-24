#pragma once
#include "Scenario.h"
#include "imgui.h"
#include <vector>
#include "Agent.h"

struct ReconObject {
    ImVec2 pos;
    float radius;
    int priority;
    bool found = false;
};

class Reconnaissance : public Scenario {
public:
    Reconnaissance(ImVec2 fieldSize);

    const char* getName() const override;
    void reset(std::vector<Agent>& agents) override;
    void update(float deltaTime, std::vector<Agent>& agents) override;
    bool isFinished() const override;
    void draw(const std::vector<Agent>&agents, float xOffset, float widthScale) override;

private:
    ImVec2 m_fieldSize;
    std::vector<ReconObject> m_objects;
    bool m_finished;
};
