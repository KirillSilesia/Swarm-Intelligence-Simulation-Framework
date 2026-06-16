#include "PathPlanning.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stack>

PathPlanning::PathPlanning(int width, int height)
    : m_width(width), m_height(height), m_finished(false) {
}

const char* PathPlanning::getName() const { return "Path Planning (Maze)"; }

// ---- Maze generation (DFS recursive backtracker) ---------------------------

void PathPlanning::generateMaze() {
    m_cells.assign(m_width * m_height, Cell{});
    std::stack<int> stk;
    int cur = 0;
    m_cells[0].visited = true;
    stk.push(0);

    while (!stk.empty()) {
        int c = stk.top();
        int x = c % m_width, y = c / m_width;

        std::vector<int> nbrs;
        if (y > 0 && !m_cells[index(x, y - 1)].visited) nbrs.push_back(0);
        if (x < m_width - 1 && !m_cells[index(x + 1, y)].visited) nbrs.push_back(1);
        if (y < m_height - 1 && !m_cells[index(x, y + 1)].visited) nbrs.push_back(2);
        if (x > 0 && !m_cells[index(x - 1, y)].visited) nbrs.push_back(3);

        if (nbrs.empty()) { stk.pop(); continue; }

        int dir = nbrs[rand() % nbrs.size()];
        int next;
        switch (dir) {
        case 0: next = index(x, y - 1); break;
        case 1: next = index(x + 1, y); break;
        case 2: next = index(x, y + 1); break;
        default:next = index(x - 1, y); break;
        }
        m_cells[c].walls[dir] = false;
        m_cells[next].walls[(dir + 2) % 4] = false;
        m_cells[next].visited = true;
        stk.push(next);
    }
}

// ---- Scenario interface ----------------------------------------------------

float PathPlanning::evaluateFitness(float x, float y) const {
    // Simple Euclidean distance to goal corner
    float gx = getGoalX(), gy = getGoalY();
    return std::hypot(x - gx, y - gy);
}

bool PathPlanning::canMoveTo(float x1, float y1, float x2, float y2) const {
    if (x2 < 0 || x2 > 1 || y2 < 0 || y2 > 1) return false;

    int cx1 = std::clamp((int)(x1 * m_width), 0, m_width - 1);
    int cy1 = std::clamp((int)(y1 * m_height), 0, m_height - 1);
    int cx2 = std::clamp((int)(x2 * m_width), 0, m_width - 1);
    int cy2 = std::clamp((int)(y2 * m_height), 0, m_height - 1);

    if (cx1 == cx2 && cy1 == cy2) return true;

    const Cell& c = m_cells[index(cx1, cy1)];
    if (cx2 > cx1 && c.walls[1]) return false;
    if (cx2 < cx1 && c.walls[3]) return false;
    if (cy2 > cy1 && c.walls[2]) return false;
    if (cy2 < cy1 && c.walls[0]) return false;
    return true;
}

// ---- Lifecycle -------------------------------------------------------------

void PathPlanning::reset(std::vector<Agent>& agents) {
    m_finished = false;
    generateMaze();

    float sx = getStartX(), sy = getStartY();
    for (auto& a : agents) {
        a.x = sx; a.y = sy;
        a.vx = a.vy = 0.0f;
        a.alive = true; a.atGoal = false;
        a.bestFitness = 1e9f;
        a.trial = 0;
        a.weight = 1.0f;
        a.distTraveled = a.timeAlive = 0.0f;
    }
    totalAgents = (int)agents.size();
}

void PathPlanning::update(float /*dt*/, std::vector<Agent>& agents) {
    // Count agents at goal
    agentsAtGoal = 0;
    for (auto& a : agents) {
        if (a.alive && !a.atGoal && isAtGoal(a.x, a.y))
            a.atGoal = true;
        if (a.atGoal) ++agentsAtGoal;
    }
    if (agentsAtGoal >= (int)(totalAgents * 0.5f) && totalAgents > 0)
        m_finished = true;
}

bool PathPlanning::isFinished() const { return m_finished; }

// ---- Drawing ---------------------------------------------------------------

void PathPlanning::draw(const std::vector<Agent>& agents,
    float xOffset, float widthScale)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float wh = io.DisplaySize.y;
    float guiH = wh / 3.0f;
    float availH = wh - guiH;

    float cellSize = std::min(18.0f * widthScale,
        (availH * 0.9f) / m_height);
    float mazeW = m_width * cellSize;
    float mazeH = m_height * cellSize;
    float yOff = guiH + (availH - mazeH) * 0.5f;
    ImVec2 O(xOffset + 20.0f, yOff);

    // Draw pheromone-like visited heatmap (faint)
    for (int cy = 0; cy < m_height; ++cy)
        for (int cx = 0; cx < m_width; ++cx) {
            ImVec2 p(O.x + cx * cellSize, O.y + cy * cellSize);
            // fill cell background
            dl->AddRectFilled(p, ImVec2(p.x + cellSize, p.y + cellSize),
                IM_COL32(30, 30, 35, 255));
        }

    // Draw walls
    for (int cy = 0; cy < m_height; ++cy) {
        for (int cx = 0; cx < m_width; ++cx) {
            const Cell& c = m_cells[index(cx, cy)];
            ImVec2 p(O.x + cx * cellSize, O.y + cy * cellSize);
            ImU32 wc = IM_COL32(200, 200, 220, 255);
            float t = 1.5f;
            if (c.walls[0]) dl->AddLine(p, ImVec2(p.x + cellSize, p.y), wc, t);
            if (c.walls[1]) dl->AddLine(ImVec2(p.x + cellSize, p.y), ImVec2(p.x + cellSize, p.y + cellSize), wc, t);
            if (c.walls[2]) dl->AddLine(ImVec2(p.x, p.y + cellSize), ImVec2(p.x + cellSize, p.y + cellSize), wc, t);
            if (c.walls[3]) dl->AddLine(p, ImVec2(p.x, p.y + cellSize), wc, t);
        }
    }

    // Green start square
    dl->AddRectFilled(O, ImVec2(O.x + cellSize, O.y + cellSize),
        IM_COL32(0, 200, 80, 180));
    dl->AddText(ImVec2(O.x + 2, O.y + 2), IM_COL32(255, 255, 255, 220), "S");

    // Red goal square
    ImVec2 gp(O.x + (m_width - 1) * cellSize, O.y + (m_height - 1) * cellSize);
    dl->AddRectFilled(gp, ImVec2(gp.x + cellSize, gp.y + cellSize),
        IM_COL32(220, 40, 40, 200));
    dl->AddText(ImVec2(gp.x + 2, gp.y + 2), IM_COL32(255, 255, 255, 220), "G");

    // Agents
    for (const auto& a : agents) {
        if (!a.alive) continue;
        ImVec2 p(O.x + a.x * mazeW, O.y + a.y * mazeH);
        ImU32 col = a.atGoal ? IM_COL32(255, 220, 0, 255) : IM_COL32(60, 200, 255, 230);
        dl->AddCircleFilled(p, 3.0f, col);
    }
}