#include "Reconnaissance.h"
#include <cstdlib>

Reconnaissance::Reconnaissance(ImVec2 topLeft, ImVec2 bottomRight)
    : m_corridorTopLeft(topLeft), m_corridorBottomRight(bottomRight), m_finished(false)
{
    reset();
}

const char* Reconnaissance::getName() const {
    return "Reconnaissance";
}

void Reconnaissance::reset() {
    m_finished = false;
    m_objects.clear();

    m_objects.push_back(ReconObject(ImVec2(50, 50), 10.0f, 3));
    m_objects.push_back(ReconObject(ImVec2(150, 100), 10.0f, 3));

    m_objects.push_back(ReconObject(ImVec2(70, 200), 8.0f, 2));
    m_objects.push_back(ReconObject(ImVec2(120, 220), 8.0f, 2));
    m_objects.push_back(ReconObject(ImVec2(200, 180), 8.0f, 2));

    m_objects.push_back(ReconObject(ImVec2(40, 300), 5.0f, 1));
    m_objects.push_back(ReconObject(ImVec2(90, 320), 5.0f, 1));
    m_objects.push_back(ReconObject(ImVec2(160, 280), 5.0f, 1));
    m_objects.push_back(ReconObject(ImVec2(220, 310), 5.0f, 1));
}

void Reconnaissance::update(float deltaTime) {
}

bool Reconnaissance::isFinished() const {
    return m_finished;
}

void Reconnaissance::draw() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    drawList->AddRectFilled(m_corridorTopLeft, m_corridorBottomRight, IM_COL32(50, 50, 50, 200));

    for (const auto& obj : m_objects) {
        ImU32 color;
        if (obj.priority == 3) color = IM_COL32(255, 0, 0, 255);
        else if (obj.priority == 2) color = IM_COL32(255, 255, 0, 255);
        else color = IM_COL32(0, 255, 0, 255);

        ImVec2 drawPos = ImVec2(
            m_corridorTopLeft.x + obj.pos.x * (m_corridorBottomRight.x - m_corridorTopLeft.x),
            m_corridorTopLeft.y + obj.pos.y * (m_corridorBottomRight.y - m_corridorTopLeft.y)
        );

        drawList->AddCircleFilled(drawPos, obj.radius, color);
    }
}
