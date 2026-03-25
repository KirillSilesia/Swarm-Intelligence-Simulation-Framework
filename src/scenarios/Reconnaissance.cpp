#include "Reconnaissance.h"
#include "Agent.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <vector>

using namespace std;

Reconnaissance::Reconnaissance(ImVec2 fieldSize)
    : m_fieldSize(fieldSize), m_finished(false)
{
}

const char* Reconnaissance::getName() const {
    return "Reconnaissance";
}

void Reconnaissance::reset(std::vector<Agent>& agents) {
    m_finished = false;
    m_objects.clear();
    for (auto& a : agents) {
        a.x = 0.0f;
        a.y = 0.5f;
        a.vx = 0.0f;
        a.vy = 0.0f;
    }

    auto rnd = []() { return (float)rand() / RAND_MAX; };
    for (int i = 0; i < 2; ++i)
        m_objects.push_back({ ImVec2(rnd(), rnd()), 8.0f, 3 });
    for (int i = 0; i < 3; ++i)
        m_objects.push_back({ ImVec2(rnd(), rnd()), 6.0f, 2 });
    for (int i = 0; i < 4; ++i)
        m_objects.push_back({ ImVec2(rnd(), rnd()), 4.0f, 1 });
}

void Reconnaissance::update(float /*dt*/, vector<Agent>& agents) {
    float detectionRadius = 0.05f;

    for (auto& obj : m_objects) {
        if (obj.found) continue;

        for (const auto& agent : agents) {
            float dx = agent.x - obj.pos.x;
            float dy = agent.y - obj.pos.y;

            float dist = sqrt(dx * dx + dy * dy);

            if (dist < detectionRadius) {
                obj.found = true;
                break;
            }
        }
    }

    m_finished = true;
    for (const auto& obj : m_objects) {
        if (!obj.found) {
            m_finished = false;
            break;
        }
    }
}

bool Reconnaissance::isFinished() const {
    return m_finished;
}

void Reconnaissance::draw(const vector<Agent>& agents) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float windowHeight = io.DisplaySize.y;
    float guiHeight = windowHeight / 3.0f;
    float availableHeight = windowHeight - guiHeight;

    float fieldWidth = m_fieldSize.x;
    float fieldHeight = m_fieldSize.y;
    float yOffset = guiHeight + (availableHeight - fieldHeight) / 2.0f;

    ImVec2 origin(20, yOffset);

    ImVec2 topLeft = origin;
    ImVec2 bottomRight(
        origin.x + fieldWidth,
        origin.y + fieldHeight
    );

    drawList->AddRectFilled(
        topLeft,
        bottomRight,
        IM_COL32(50, 50, 50, 200)
    );

    for (const auto& obj : m_objects) {
        ImU32 color;

        if (obj.found)
            color = IM_COL32(100, 100, 100, 255);
        else if (obj.priority == 3)
            color = IM_COL32(255, 0, 0, 255);
        else if (obj.priority == 2)
            color = IM_COL32(255, 255, 0, 255);
        else
            color = IM_COL32(0, 255, 0, 255);
        ImVec2 pos(
            topLeft.x + obj.pos.x * fieldWidth,
            topLeft.y + obj.pos.y * fieldHeight
        );
        drawList->AddCircleFilled(pos, obj.radius, color);
    }

    drawList->AddRect(
        topLeft,
        bottomRight,
        IM_COL32(180, 180, 180, 255),
        0.0f,
        0,
        2.0f
    );

    float markerSize = 16.0f;

    float centerY = topLeft.y + fieldHeight / 2.0f;

    ImVec2 p1(topLeft.x - markerSize / 2, centerY - markerSize / 2);
    ImVec2 p2(topLeft.x + markerSize / 2, centerY - markerSize / 2);
    ImVec2 p3(topLeft.x + markerSize / 2, centerY + markerSize / 2);
    ImVec2 p4(topLeft.x - markerSize / 2, centerY + markerSize / 2);


    drawList->AddTriangleFilled(p1, p2, p4, IM_COL32(255, 0, 0, 255));

    drawList->AddTriangleFilled(p2, p3, p4, IM_COL32(0, 255, 0, 255));

    drawList->AddRect(p1, p3, IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f);

    for (const auto& a : agents) {
        ImVec2 p(
            topLeft.x + a.x * fieldWidth,
            topLeft.y + a.y * fieldHeight
        );

        drawList->AddCircleFilled(p, 3.0f, IM_COL32(0, 200, 255, 255));
    }
}