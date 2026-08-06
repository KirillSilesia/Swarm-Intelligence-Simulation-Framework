#include "Reconnaissance.h"
#include "Agent.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

static float frand01() { return (float)rand() / (float)RAND_MAX; }

Reconnaissance::Reconnaissance(ImVec2 fieldSize)
    : m_fieldSize(fieldSize), m_finished(false) {
}

const char* Reconnaissance::getName() const { return "Reconnaissance"; }


float Reconnaissance::evaluateFitness(float x, float y) const {
    for (int pri = 3; pri >= 1; --pri) {
        bool anyLeft = false;
        float best = 1e9f;
        for (const auto& obj : m_objects) {
            if (obj.found || obj.priority != pri) continue;
            anyLeft = true;
            float d = std::hypot(x - obj.x, y - obj.y);
            float weighted = d / (float)pri;
            if (weighted < best) best = weighted;
        }
        if (anyLeft) return best;
    }
    return std::hypot(x - getGoalX(), y - getGoalY());
}


void Reconnaissance::reset(std::vector<Agent>& agents) {
    m_finished = false;
    m_objects.clear();
    elapsedTime = 0.0f;
    agentsAtGoal = 0;
    totalAgents = (int)agents.size();

    for (int i = 0; i < 2; ++i)
        m_objects.push_back({ 0.2f + frand01() * 0.6f, 0.2f + frand01() * 0.6f,
                               10.0f, 3, false, -1.0f });

    for (int i = 0; i < 3; ++i)
        m_objects.push_back({ frand01() * 0.9f + 0.05f, frand01() * 0.9f + 0.05f,
                               7.0f, 2, false, -1.0f });

    for (int i = 0; i < 4; ++i)
        m_objects.push_back({ frand01() * 0.9f + 0.05f, frand01() * 0.9f + 0.05f,
                               5.0f, 1, false, -1.0f });

    float sx = getStartX(), sy = getStartY();
    for (auto& a : agents) {
        a.x = sx + (frand01() - 0.5f) * 0.04f;
        a.y = sy + (frand01() - 0.5f) * 0.04f;
        a.x = std::clamp(a.x, 0.0f, 1.0f);
        a.y = std::clamp(a.y, 0.0f, 1.0f);
        a.vx = a.vy = 0.0f;
        a.alive = true; a.atGoal = false;
        a.bestFitness = 1e9f; a.trial = 0;
        a.weight = 1.0f; a.distTraveled = a.timeAlive = 0.0f;
    }
}

void Reconnaissance::update(float dt, std::vector<Agent>& agents) {
    elapsedTime += dt;
    const float detectR = 0.06f;

    for (auto& obj : m_objects) {
        if (obj.found) continue;
        for (const auto& a : agents) {
            if (!a.alive) continue;
            float d = std::hypot(a.x - obj.x, a.y - obj.y);
            if (d < detectR) {
                obj.found = true;
                obj.foundTime = elapsedTime;
                break;
            }
        }
    }

    agentsAtGoal = 0;
    for (auto& a : agents) {
        if (a.alive && !a.atGoal && isAtGoal(a.x, a.y))
            a.atGoal = true;
        if (a.atGoal) ++agentsAtGoal;
    }

    bool allHighFound = true;
    for (const auto& obj : m_objects)
        if (obj.priority == 3 && !obj.found) { allHighFound = false; break; }

    if (allHighFound && agentsAtGoal >= (int)(totalAgents * 0.5f) && totalAgents > 0)
        m_finished = true;
}

bool Reconnaissance::isFinished() const { return m_finished; }

int Reconnaissance::highFound() const {
    int c = 0; for (const auto& o : m_objects) if (o.priority == 3 && o.found) ++c; return c;
}
int Reconnaissance::medFound() const {
    int c = 0; for (const auto& o : m_objects) if (o.priority == 2 && o.found) ++c; return c;
}
int Reconnaissance::lowFound() const {
    int c = 0; for (const auto& o : m_objects) if (o.priority == 1 && o.found) ++c; return c;
}


void Reconnaissance::draw(const std::vector<Agent>& agents,
    float xOffset, float widthScale)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float wh = io.DisplaySize.y;
    float guiH = wh / 3.0f;
    float availH = wh - guiH;

    float fieldW = m_fieldSize.x * widthScale;
    float fieldH = m_fieldSize.y;
    if (fieldH > availH * 0.9f) fieldH = availH * 0.9f;
    fieldW = fieldH;

    float yOff = guiH + (availH - fieldH) * 0.5f;
    ImVec2 TL(xOffset + 20.0f, yOff);
    ImVec2 BR(TL.x + fieldW, TL.y + fieldH);

    viewX = TL.x; viewY = TL.y; viewW = fieldW; viewH = fieldH;

    dl->AddRectFilled(TL, BR, IM_COL32(20, 25, 20, 220));
    dl->AddRect(TL, BR, IM_COL32(100, 120, 100, 255), 0.0f, 0, 2.0f);

    for (const auto& obj : m_objects) {
        float px = TL.x + obj.x * fieldW;
        float py = TL.y + obj.y * fieldH;
        if (obj.priority == 3 && !obj.found) {
            dl->AddCircle(ImVec2(px, py), obj.radius * 3.5f,
                IM_COL32(255, 80, 80, 30), 32, 1.0f);
            dl->AddCircle(ImVec2(px, py), obj.radius * 2.0f,
                IM_COL32(255, 80, 80, 50), 32, 1.0f);
        }
        if (obj.priority == 1 && !obj.found) {
            dl->AddCircle(ImVec2(px, py), obj.radius * 1.5f,
                IM_COL32(80, 255, 80, 25), 12, 1.0f);
        }
    }

    for (const auto& obj : m_objects) {
        float px = TL.x + obj.x * fieldW;
        float py = TL.y + obj.y * fieldH;
        ImU32 col;
        if (obj.found)       col = IM_COL32(80, 80, 80, 160);
        else if (obj.priority == 3) col = IM_COL32(255, 60, 60, 230);
        else if (obj.priority == 2) col = IM_COL32(255, 220, 0, 220);
        else                         col = IM_COL32(60, 220, 60, 200);

        dl->AddCircleFilled(ImVec2(px, py), obj.radius, col);
        dl->AddCircle(ImVec2(px, py), obj.radius, IM_COL32(255, 255, 255, 120), 0, 1.0f);

        // Priority label
        char lbl[4]; lbl[0] = 'P'; lbl[1] = '0' + (char)obj.priority; lbl[2] = '\0';
        dl->AddText(ImVec2(px - 6, py - 5), IM_COL32(255, 255, 255, 200), lbl);
    }

    float sqSz = 20.0f;
    float sxPx = TL.x + getStartX() * fieldW;
    float syPx = TL.y + getStartY() * fieldH;
    dl->AddRectFilled(ImVec2(sxPx - sqSz / 2, syPx - sqSz / 2),
        ImVec2(sxPx + sqSz / 2, syPx + sqSz / 2),
        IM_COL32(0, 200, 80, 200));
    dl->AddText(ImVec2(sxPx - 6, syPx - 6), IM_COL32(255, 255, 255, 240), "S");

    float gxPx = TL.x + getGoalX() * fieldW;
    float gyPx = TL.y + getGoalY() * fieldH;
    dl->AddRectFilled(ImVec2(gxPx - sqSz / 2, gyPx - sqSz / 2),
        ImVec2(gxPx + sqSz / 2, gyPx + sqSz / 2),
        IM_COL32(220, 40, 40, 200));
    dl->AddText(ImVec2(gxPx - 6, gyPx - 6), IM_COL32(255, 255, 255, 240), "G");

    for (const auto& a : agents) {
        if (!a.alive) continue;
        float px = TL.x + a.x * fieldW;
        float py = TL.y + a.y * fieldH;
        ImU32 col = a.atGoal ? IM_COL32(255, 220, 0, 255) : IM_COL32(60, 200, 255, 230);
        dl->AddCircleFilled(ImVec2(px, py), 3.5f, col);
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "High: %d/2  Med: %d/3  Low: %d/4  Goal: %d/%d",
        highFound(), medFound(), lowFound(), agentsAtGoal, totalAgents);
    dl->AddText(ImVec2(TL.x + 4, TL.y + 4), IM_COL32(220, 220, 220, 220), buf);
}