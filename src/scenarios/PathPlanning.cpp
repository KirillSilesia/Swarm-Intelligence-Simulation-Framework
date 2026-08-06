#include "PathPlanning.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <queue>
#include <stack>

PathPlanning::PathPlanning(int width, int height)
    : m_width(width), m_height(height), m_finished(false) {
}

const char* PathPlanning::getName() const { return "Path Planning (Maze)"; }


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

    computeDistanceField();
}


// Breadth-first flood from the goal cell, stepping only through open walls.
// The result is the true corridor distance from each cell to the goal, which
// (unlike straight-line distance) decreases monotonically along every path
// that actually reaches the goal -- so agents can never get trapped in a
// corner that merely looks close.
void PathPlanning::computeDistanceField() {
    m_dist.assign(m_width * m_height, -1);

    int gx = m_width - 1, gy = m_height - 1;
    std::queue<int> q;
    m_dist[index(gx, gy)] = 0;
    q.push(index(gx, gy));

    static const int dx4[] = { 1, -1, 0, 0 };
    static const int dy4[] = { 0, 0, 1, -1 };

    while (!q.empty()) {
        int c = q.front(); q.pop();
        int x = c % m_width, y = c / m_width;
        for (int d = 0; d < 4; ++d) {
            int nx = x + dx4[d], ny = y + dy4[d];
            if (nx < 0 || nx >= m_width || ny < 0 || ny >= m_height) continue;
            if (m_dist[index(nx, ny)] != -1) continue;
            if (!wallOpen(x, y, nx, ny)) continue;
            m_dist[index(nx, ny)] = m_dist[c] + 1;
            q.push(index(nx, ny));
        }
    }
}


float PathPlanning::evaluateFitness(float x, float y) const {
    int cx = std::clamp((int)(x * m_width), 0, m_width - 1);
    int cy = std::clamp((int)(y * m_height), 0, m_height - 1);

    int d = m_dist.empty() ? -1 : m_dist[index(cx, cy)];
    if (d < 0) return 1e6f;  // isolated cell (should not occur in a perfect maze)

    // Corridor distance (in cells) dominates; a tiny straight-line term breaks
    // ties within a cell and gives continuous-space methods a gentle gradient.
    // Its variation between adjacent cells (~one cell width) stays well below 1,
    // so it can never reorder two cells against the geodesic ranking.
    float euclid = std::hypot(x - getGoalX(), y - getGoalY());
    return (float)d + euclid;
}

bool PathPlanning::guidanceDir(float x, float y, float& dirX, float& dirY) const {
    dirX = dirY = 0.0f;
    if (m_dist.empty()) return false;

    int cx = std::clamp((int)(x * m_width), 0, m_width - 1);
    int cy = std::clamp((int)(y * m_height), 0, m_height - 1);
    int here = m_dist[index(cx, cy)];
    if (here <= 0) return false;  // already in the goal cell (or isolated)

    // Follow the BFS field: head for the open neighbour that is closest to the
    // goal. Steering toward that cell's centre also keeps agents off the walls.
    static const int dx4[] = { 1, -1, 0, 0 };
    static const int dy4[] = { 0, 0, 1, -1 };

    int best = here, bnx = -1, bny = -1;
    for (int d = 0; d < 4; ++d) {
        int nx = cx + dx4[d], ny = cy + dy4[d];
        if (nx < 0 || nx >= m_width || ny < 0 || ny >= m_height) continue;
        if (!wallOpen(cx, cy, nx, ny)) continue;
        int nd = m_dist[index(nx, ny)];
        if (nd >= 0 && nd < best) { best = nd; bnx = nx; bny = ny; }
    }
    if (bnx < 0) return false;

    float tcx = (bnx + 0.5f) / m_width;
    float tcy = (bny + 0.5f) / m_height;
    float ux = tcx - x, uy = tcy - y;
    float len = std::hypot(ux, uy) + 1e-9f;
    dirX = ux / len;
    dirY = uy / len;
    return true;
}

bool PathPlanning::wallOpen(int fx, int fy, int tx, int ty) const {
    const Cell& c = m_cells[index(fx, fy)];
    if (tx > fx) return !c.walls[1];
    if (tx < fx) return !c.walls[3];
    if (ty > fy) return !c.walls[2];
    if (ty < fy) return !c.walls[0];
    return true;
}

bool PathPlanning::canMoveTo(float x1, float y1, float x2, float y2) const {
    if (x2 < 0 || x2 > 1 || y2 < 0 || y2 > 1) return false;

    float dx = x2 - x1, dy = y2 - y1;
    float dist = std::hypot(dx, dy);

    int steps = std::max(1, (int)std::ceil(dist * std::max(m_width, m_height) * 4.0f));

    int pcx = std::clamp((int)(x1 * m_width), 0, m_width - 1);
    int pcy = std::clamp((int)(y1 * m_height), 0, m_height - 1);

    for (int s = 1; s <= steps; ++s) {
        float t = (float)s / steps;
        float sx = x1 + dx * t, sy = y1 + dy * t;
        int cx = std::clamp((int)(sx * m_width), 0, m_width - 1);
        int cy = std::clamp((int)(sy * m_height), 0, m_height - 1);

        if (cx == pcx && cy == pcy) continue;

        if (cx != pcx && cy != pcy) {
            bool viaX = wallOpen(pcx, pcy, cx, pcy) && wallOpen(cx, pcy, cx, cy);
            bool viaY = wallOpen(pcx, pcy, pcx, cy) && wallOpen(pcx, cy, cx, cy);
            if (!viaX && !viaY) return false;
        }
        else {
            if (!wallOpen(pcx, pcy, cx, cy)) return false;
        }

        pcx = cx; pcy = cy;
    }
    return true;
}


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

    viewX = O.x; viewY = O.y; viewW = mazeW; viewH = mazeH;

    for (int cy = 0; cy < m_height; ++cy)
        for (int cx = 0; cx < m_width; ++cx) {
            ImVec2 p(O.x + cx * cellSize, O.y + cy * cellSize);
            dl->AddRectFilled(p, ImVec2(p.x + cellSize, p.y + cellSize),
                IM_COL32(30, 30, 35, 255));
        }

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

    dl->AddRectFilled(O, ImVec2(O.x + cellSize, O.y + cellSize),
        IM_COL32(0, 200, 80, 180));
    dl->AddText(ImVec2(O.x + 2, O.y + 2), IM_COL32(255, 255, 255, 220), "S");

    ImVec2 gp(O.x + (m_width - 1) * cellSize, O.y + (m_height - 1) * cellSize);
    dl->AddRectFilled(gp, ImVec2(gp.x + cellSize, gp.y + cellSize),
        IM_COL32(220, 40, 40, 200));
    dl->AddText(ImVec2(gp.x + 2, gp.y + 2), IM_COL32(255, 255, 255, 220), "G");

    for (const auto& a : agents) {
        if (!a.alive) continue;
        ImVec2 p(O.x + a.x * mazeW, O.y + a.y * mazeH);
        ImU32 col = a.atGoal ? IM_COL32(255, 220, 0, 255) : IM_COL32(60, 200, 255, 230);
        dl->AddCircleFilled(p, 3.0f, col);
    }
}