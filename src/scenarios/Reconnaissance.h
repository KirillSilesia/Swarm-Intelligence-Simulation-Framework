#pragma once
#include "Scenario.h"
#include "imgui.h"
#include <vector>

struct ReconObject {
    ImVec2 pos;
    float radius;
    int priority;

    ReconObject(const ImVec2& p, float r, int prio)
        : pos(p), radius(r), priority(prio) {
    }
};

class Reconnaissance : public Scenario {
public:
    Reconnaissance(int width, int height);

    const char* getName() const override;
    void reset() override;
    void update(float deltaTime) override;
    bool isFinished() const override;
    void draw() override;

private:
    int m_width;
    int m_height;
    bool m_finished;
    std::vector<ReconObject> m_objects;
};
