#pragma once
#include "Scenario.h"
#include "imgui.h"
#include <vector>

struct ReconObject {
    ImVec2 pos;
    float radius;
    int priority;
};

class Reconnaissance : public Scenario {
public:
    Reconnaissance(ImVec2 fieldSize);

    const char* getName() const override;
    void reset() override;
    void update(float deltaTime) override;
    bool isFinished() const override;
    void draw() override;

private:
    ImVec2 m_fieldSize;
    std::vector<ReconObject> m_objects;
    bool m_finished;
};
