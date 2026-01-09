#include "Reconnaissance.h"
#include <cstdlib>

Reconnaissance::Reconnaissance(ImVec2 fieldSize)
    : m_fieldSize(fieldSize), m_finished(false)
{
    reset();
}

const char* Reconnaissance::getName() const {
    return "Reconnaissance";
}

void Reconnaissance::reset() {
    m_finished = false;
    m_objects.clear();
    auto rnd = []() { return (float)rand() / RAND_MAX; };
    for (int i = 0; i < 2; ++i)
        m_objects.push_back({ ImVec2(rnd(), rnd()), 8.0f, 3 });
    for (int i = 0; i < 3; ++i)
        m_objects.push_back({ ImVec2(rnd(), rnd()), 6.0f, 2 });
    for (int i = 0; i < 4; ++i)
        m_objects.push_back({ ImVec2(rnd(), rnd()), 4.0f, 1 });
}

void Reconnaissance::update(float) {}

bool Reconnaissance::isFinished() const {
    return m_finished;
}

void Reconnaissance::draw() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float windowHeight = io.DisplaySize.y;
    float guiHeight = windowHeight / 3.0f;
    float availableHeight = windowHeight - guiHeight;

    float fieldWidth = m_fieldSize.x;
    float fieldHeight = m_fieldSize.y;
    float yOffset = guiHeight + (availableHeight - fieldHeight) / 2.0f;

    // Match PathPlanning's origin calculation
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
        ImU32 color =
            obj.priority == 3 ? IM_COL32(255, 0, 0, 255) :
            obj.priority == 2 ? IM_COL32(255, 255, 0, 255) :
            IM_COL32(0, 255, 0, 255);
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
}