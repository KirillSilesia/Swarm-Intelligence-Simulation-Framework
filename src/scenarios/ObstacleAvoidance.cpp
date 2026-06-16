/*
 * ObstacleAvoidance scenario.
 *
 * Corridor layout (normalized coords):
 *   Start (GREEN) at LEFT  (x≈0.03, y≈0.5)
 *   Goal  (RED)   at RIGHT (x≈0.97, y≈0.5)
 *
 * Agents navigate left→right through moving obstacles.
 * Collision with an obstacle → agent.alive = false (agent disappears).
 * Finish: ≥50% of original agents reached the right-side goal, or all dead.
 */
#include "ObstacleAvoidance.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

static float frand() { return (float)rand() / (float)RAND_MAX; }

ObstacleAvoidance::ObstacleAvoidance(int w, int h)
    : m_width(w), m_height(h) {
}

const char* ObstacleAvoidance::getName() const { return "Obstacle Avoidance"; }

// ---- Obstacle generation ---------------------------------------------------

void ObstacleAvoidance::generateObstacles() {
    m_obstacles.clear();
    int n = 8 + rand() % 6;
    for (int i = 0; i < n; ++i) {
        MovingObstacle o;
        o.rx = 0.15f + frand() * 0.7f;
        o.ry = 0.1f + frand() * 0.8f;
        float speedX = -(0.03f + frand() * 0.07f);   // normalized/s
        float speedY = (frand() - 0.5f) * 0.04f;
        o.vx = speedX; o.vy = speedY;
        o.displaySize = 12.0f + frand() * 20.0f;
        o.normSize = o.displaySize / m_corridorW;
        o.shape = rand() % 3;
        o.rotation = frand() * 6.28f;
        o.rotSpeed = (frand() - 0.5f) * 2.0f;
        m_obstacles.push_back(o);
    }
}

// ---- Scenario interface ----------------------------------------------------

float ObstacleAvoidance::evaluateFitness(float x, float y) const {
    // Minimize: want to move right, so base = (1-x)
    float base = 1.0f - x;
    // Penalty for being close to obstacles
    for (const auto& o : m_obstacles) {
        float dx = x - o.rx, dy = y - o.ry;
        float d = std::hypot(dx, dy);
        float safe = o.normSize + 0.04f;
        if (d < safe) base += (safe - d) * 8.0f;
    }
    return base;
}

bool ObstacleAvoidance::canMoveTo(float /*x1*/, float /*y1*/,
    float x2, float y2) const {
    if (x2 < 0.0f || x2 > 1.0f) return false;
    if (y2 < 0.0f || y2 > 1.0f) return false;
    // Entry/exit gaps are centred at y=0.5
    float gapNorm = m_gapHalf / m_corridorH;
    if (x2 < 0.02f) {
        // Left wall – must be in gap
        if (std::abs(y2 - 0.5f) > gapNorm) return false;
    }
    if (x2 > 0.98f) {
        // Right wall – must be in gap
        if (std::abs(y2 - 0.5f) > gapNorm) return false;
    }
    return true;
}

// ---- Lifecycle -------------------------------------------------------------

void ObstacleAvoidance::reset(std::vector<Agent>& agents) {
    m_finished = false;
    m_collisions = 0;
    generateObstacles();

    float sx = getStartX(), sy = getStartY();
    for (auto& a : agents) {
        a.x = sx + (frand() - 0.5f) * 0.02f;
        a.y = sy + (frand() - 0.5f) * 0.08f;
        a.x = std::clamp(a.x, 0.0f, 1.0f);
        a.y = std::clamp(a.y, 0.0f, 1.0f);
        a.vx = a.vy = 0.0f;
        a.alive = true; a.atGoal = false;
        a.bestFitness = 1e9f; a.trial = 0;
        a.weight = 1.0f; a.distTraveled = a.timeAlive = 0.0f;
    }
    totalAgents = (int)agents.size();
    elapsedTime = 0.0f;
    agentsAtGoal = 0;
}

void ObstacleAvoidance::update(float dt, std::vector<Agent>& agents) {
    elapsedTime += dt;

    // 1. Move obstacles
    for (auto& o : m_obstacles) {
        o.rx += o.vx * dt;
        o.ry += o.vy * dt;
        o.rotation += o.rotSpeed * dt;

        // Bounce off top/bottom
        if (o.ry < 0.05f) { o.ry = 0.05f; o.vy = std::abs(o.vy); }
        if (o.ry > 0.95f) { o.ry = 0.95f; o.vy = -std::abs(o.vy); }

        // Wrap around left edge → respawn on right
        if (o.rx < -0.1f) {
            o.rx = 1.05f + frand() * 0.3f;
            o.ry = 0.1f + frand() * 0.8f;
        }
    }

    // 2. Collision detection – kill agents that touch obstacles
    for (auto& a : agents) {
        if (!a.alive || a.atGoal) continue;
        for (const auto& o : m_obstacles) {
            float dx = a.x - o.rx, dy = a.y - o.ry;
            float d = std::hypot(dx, dy);
            if (d < o.normSize * 1.1f) {
                a.alive = false;
                ++m_collisions;
                break;
            }
        }
    }

    // 3. Goal check (reached right side within gap)
    agentsAtGoal = 0;
    float gapNorm = m_gapHalf / m_corridorH;
    for (auto& a : agents) {
        if (!a.alive) continue;
        if (a.x >= 0.95f && std::abs(a.y - 0.5f) <= gapNorm)
            a.atGoal = true;
        if (a.atGoal) ++agentsAtGoal;
    }

    // 4. Finish condition
    int alive = 0;
    for (const auto& a : agents) if (a.alive) ++alive;
    if (agentsAtGoal >= (int)(totalAgents * 0.5f) ||
        (alive == 0 && totalAgents > 0))
        m_finished = true;
}

bool ObstacleAvoidance::isFinished() const { return m_finished; }

// ---- Drawing ---------------------------------------------------------------

void ObstacleAvoidance::draw(const std::vector<Agent>& agents,
    float xOffset, float widthScale)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float wh = io.DisplaySize.y;
    float guiH = wh / 3.0f;
    float availH = wh - guiH;

    float cW = m_corridorW * widthScale;
    float cH = std::min(m_corridorH, availH * 0.85f);
    float yOff = guiH + (availH - cH) * 0.5f;
    float gapPx = m_gapHalf / m_corridorH * cH;

    ImVec2 TL(xOffset + 20.0f, yOff);
    ImVec2 BR(TL.x + cW, TL.y + cH);
    float midY = TL.y + cH * 0.5f;

    // Corridor background
    dl->AddRectFilled(TL, BR, IM_COL32(20, 20, 30, 220));

    // Clip obstacles to corridor
    dl->PushClipRect(TL, BR, true);

    // Draw obstacles
    for (const auto& o : m_obstacles) {
        ImVec2 p(TL.x + o.rx * cW, TL.y + o.ry * cH);
        float  sz = o.displaySize * widthScale;
        ImU32  fc = IM_COL32(220, 100, 20, 230);
        ImU32  oc = IM_COL32(255, 200, 100, 200);

        switch (o.shape) {
        case 0:  // circle
            dl->AddCircleFilled(p, sz, fc);
            dl->AddCircle(p, sz, oc, 0, 1.5f);
            break;
        case 1: { // rotated rectangle
            float c = std::cos(o.rotation), s = std::sin(o.rotation);
            ImVec2 corners[4] = {
                {p.x + (-sz * c - -sz * s), p.y + (-sz * s + -sz * c)},
                {p.x + (sz * c - -sz * s), p.y + (sz * s + -sz * c)},
                {p.x + (sz * c - sz * s), p.y + (sz * s + sz * c)},
                {p.x + (-sz * c - sz * s), p.y + (-sz * s + sz * c)}
            };
            dl->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], fc);
            dl->AddQuad(corners[0], corners[1], corners[2], corners[3], oc, 1.5f);
            break;
        }
        case 2: { // triangle
            float r = sz;
            ImVec2 p1(p.x + r * cosf(o.rotation), p.y + r * sinf(o.rotation));
            ImVec2 p2(p.x + r * cosf(o.rotation + 2.094f), p.y + r * sinf(o.rotation + 2.094f));
            ImVec2 p3(p.x + r * cosf(o.rotation - 2.094f), p.y + r * sinf(o.rotation - 2.094f));
            dl->AddTriangleFilled(p1, p2, p3, fc);
            dl->AddTriangle(p1, p2, p3, oc, 1.5f);
            break;
        }
        }
    }

    // Agents
    for (const auto& a : agents) {
        if (!a.alive) continue;
        ImVec2 p(TL.x + a.x * cW, TL.y + a.y * cH);
        ImU32 col = a.atGoal ? IM_COL32(255, 220, 0, 255) : IM_COL32(60, 200, 255, 230);
        dl->AddCircleFilled(p, 3.5f, col);
    }
    dl->PopClipRect();

    // Walls
    ImU32 wc = IM_COL32(160, 160, 180, 255);
    float wt = 2.5f;
    dl->AddLine(TL, ImVec2(BR.x, TL.y), wc, wt); // top
    dl->AddLine(ImVec2(TL.x, BR.y), BR, wc, wt);  // bottom
    // Left wall with gap (GREEN = START)
    dl->AddLine(TL, ImVec2(TL.x, midY - gapPx), wc, wt);
    dl->AddLine(ImVec2(TL.x, midY + gapPx), ImVec2(TL.x, BR.y), wc, wt);
    // Right wall with gap (RED = GOAL)
    dl->AddLine(ImVec2(BR.x, TL.y), ImVec2(BR.x, midY - gapPx), wc, wt);
    dl->AddLine(ImVec2(BR.x, midY + gapPx), BR, wc, wt);

    // Green start marker (LEFT)
    float ms = 16.0f;
    dl->AddRectFilled(ImVec2(TL.x - ms * 0.5f, midY - ms * 0.5f),
        ImVec2(TL.x + ms * 0.5f, midY + ms * 0.5f),
        IM_COL32(0, 200, 80, 220));
    dl->AddText(ImVec2(TL.x - 5, midY - 6), IM_COL32(255, 255, 255, 240), "S");

    // Red goal marker (RIGHT)
    dl->AddRectFilled(ImVec2(BR.x - ms * 0.5f, midY - ms * 0.5f),
        ImVec2(BR.x + ms * 0.5f, midY + ms * 0.5f),
        IM_COL32(220, 40, 40, 220));
    dl->AddText(ImVec2(BR.x - 5, midY - 6), IM_COL32(255, 255, 255, 240), "G");

    // Stats
    int alive = 0;
    for (const auto& a : agents) if (a.alive) ++alive;
    char buf[128];
    snprintf(buf, sizeof(buf), "Alive: %d/%d  Goal: %d  Collisions: %d",
        alive, totalAgents, agentsAtGoal, m_collisions);
    dl->AddText(ImVec2(TL.x + 4, TL.y + 4), IM_COL32(220, 220, 220, 220), buf);
}