#include "AntColonyOptimization.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

static float frand() { return (float)rand() / (float)RAND_MAX; }

void AntColonyOptimization::initialize(std::vector<Agent>& agents,
    Scenario& scenario)
{
    for (int y = 0; y < GRID; ++y)
        for (int x = 0; x < GRID; ++x)
            m_tau[y][x] = 0.0f;

    m_paths.resize(agents.size());
    for (auto& p : m_paths) {
        p.cells.clear();
        p.cost = 0.0f;
        p.reachedGoal = false;
    }

    float sx = scenario.getStartX();
    float sy = scenario.getStartY();

    for (int i = 0; i < (int)agents.size(); ++i) {
        agents[i].x = sx;
        agents[i].y = sy;
        agents[i].vx = agents[i].vy = 0.0f;

        int gx = (int)(sx * GRID); gx = std::clamp(gx, 0, GRID - 1);
        int gy = (int)(sy * GRID); gy = std::clamp(gy, 0, GRID - 1);
        m_paths[i].cells = { {gx, gy} };
    }
}

void AntColonyOptimization::update(std::vector<Agent>& agents,
    Scenario& scenario, float dt)
{
    for (int y = 0; y < GRID; ++y)
        for (int x = 0; x < GRID; ++x)
            m_tau[y][x] *= (1.0f - m_rho);

    static const int dx4[] = { 1, -1,  0, 0 };
    static const int dy4[] = { 0,  0,  1,-1 };

    for (int i = 0; i < (int)agents.size(); ++i) {
        auto& a = agents[i];
        if (!a.alive || a.atGoal) continue;

        int cx = (int)(a.x * GRID); cx = std::clamp(cx, 0, GRID - 1);
        int cy = (int)(a.y * GRID); cy = std::clamp(cy, 0, GRID - 1);

        float weights[4] = {};
        bool  valid[4] = {};

        float curFit = scenario.evaluateFitness(a.x, a.y);

        for (int d = 0; d < 4; ++d) {
            int nx = cx + dx4[d];
            int ny = cy + dy4[d];
            if (nx < 0 || nx >= GRID || ny < 0 || ny >= GRID) continue;

            float tx = (nx + 0.5f) / GRID;
            float ty = (ny + 0.5f) / GRID;

            if (!scenario.canMoveTo(a.x, a.y, tx, ty)) continue;

            // Desirability of a neighbour = pheromone^alpha * heuristic. The
            // heuristic rewards neighbours that *reduce* the scenario fitness
            // (i.e. step down the maze's corridor distance to the goal). Using
            // the improvement through exp() keeps the value well-scaled however
            // far the goal is -- unlike 1/fitness, which underflows at long
            // distances and collapses the ants into an unguided random walk.
            float nFit = scenario.evaluateFitness(tx, ty);
            float tau = std::pow(m_tau[ny][nx] + 1e-6f, m_alpha);
            float eta = std::exp(m_beta * (curFit - nFit));
            weights[d] = tau * eta;
            valid[d] = true;
        }

        float total = 0.0f;
        for (int d = 0; d < 4; ++d) total += weights[d];

        if (total < 1e-9f) {
            for (int d = 0; d < 4; ++d) {
                if (valid[d]) weights[d] = 1.0f;
            }
            for (int d = 0; d < 4; ++d) total += weights[d];
        }

        float r = frand() * total;
        int chosen = -1;
        float acc = 0.0f;
        for (int d = 0; d < 4; ++d) {
            acc += weights[d];
            if (r <= acc && valid[d]) { chosen = d; break; }
        }
        if (chosen == -1) {
            for (int d = 0; d < 4; ++d)
                if (valid[d]) { chosen = d; break; }
        }
        if (chosen == -1) continue;

        int nx = cx + dx4[chosen];
        int ny = cy + dy4[chosen];
        float tx = (nx + 0.5f) / GRID;
        float ty = (ny + 0.5f) / GRID;

        float ox = a.x, oy = a.y;
        a.x = tx; a.y = ty;
        a.vx = (a.x - ox) / (dt + 1e-6f);
        a.vy = (a.y - oy) / (dt + 1e-6f);
        a.distTraveled += std::hypot(a.x - ox, a.y - oy);
        a.timeAlive += dt;

        m_paths[i].cells.push_back({ nx, ny });
        m_paths[i].cost += 1.0f;

        if (scenario.isAtGoal(a.x, a.y)) {
            a.atGoal = true;
            m_paths[i].reachedGoal = true;
            float delta = m_Q / (m_paths[i].cost + 1.0f);
            for (auto& cell : m_paths[i].cells)
                m_tau[cell.second][cell.first] += delta;
        }
    }
}

void AntColonyOptimization::drawOverlay(const Scenario& scenario)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    // Map the pheromone grid onto exactly the rectangle the scenario drew, so
    // the trail sits on the maze corridors instead of floating over the panel.
    float cellW = scenario.viewW / GRID;
    float cellH = scenario.viewH / GRID;

    float maxTau = 0.0f;
    for (int y = 0; y < GRID; ++y)
        for (int x = 0; x < GRID; ++x)
            if (m_tau[y][x] > maxTau) maxTau = m_tau[y][x];
    if (maxTau < 1e-6f) return;

    for (int y = 0; y < GRID; ++y) {
        for (int x = 0; x < GRID; ++x) {
            float intensity = m_tau[y][x] / maxTau;
            if (intensity < 0.04f) continue;
            ImU32 col = IM_COL32(
                (int)(255 * intensity * 0.5f),
                (int)(200 * intensity),
                (int)(255 * intensity * 0.2f),
                (int)(180 * intensity));
            float px = scenario.viewX + x * cellW;
            float py = scenario.viewY + y * cellH;
            dl->AddRectFilled(ImVec2(px, py),
                ImVec2(px + cellW, py + cellH), col);
        }
    }
}