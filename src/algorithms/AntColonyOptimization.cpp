/*
 * ACO – grid-based pheromone map (40×40 over normalized space).
 *
 * Each ant:
 *   - Steps one grid cell per update tick.
 *   - Chooses next cell:  P(i→j) = τ[j]^α * η[j]^β / Σ
 *   - On reaching goal:   deposits Δτ = Q/path_length along its path,
 *                         marks atGoal = true.
 *
 * Every tick:  τ[i,j] ← (1-ρ) * τ[i,j]   (evaporation)
 */
#include "AntColonyOptimization.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>

static float frand() { return (float)rand() / (float)RAND_MAX; }

// ---- helpers ---------------------------------------------------------------

float AntColonyOptimization::heuristic(int gx, int gy,
    float goalX, float goalY) const
{
    float cx = (gx + 0.5f) / GRID;
    float cy = (gy + 0.5f) / GRID;
    float d = std::hypot(cx - goalX, cy - goalY);
    return 1.0f / (d + 0.001f);
}

// ---- lifecycle -------------------------------------------------------------

void AntColonyOptimization::initialize(std::vector<Agent>& agents,
    Scenario& scenario)
{
    // Initialise τ uniformly small
    for (int y = 0; y < GRID; ++y)
        for (int x = 0; x < GRID; ++x)
            m_tau[y][x] = 0.1f;

    m_paths.resize(agents.size());
    for (auto& p : m_paths) {
        p.cells.clear();
        p.cost = 0.0f;
        p.reachedGoal = false;
    }

    // Place each ant at start cell
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
    float goalX = scenario.getGoalX();
    float goalY = scenario.getGoalY();

    // 1. Evaporate pheromones on every edge
    for (int y = 0; y < GRID; ++y)
        for (int x = 0; x < GRID; ++x)
            m_tau[y][x] *= (1.0f - m_rho);

    // 2. Move each ant one step
    static const int dx4[] = { 1, -1,  0, 0 };
    static const int dy4[] = { 0,  0,  1,-1 };

    for (int i = 0; i < (int)agents.size(); ++i) {
        auto& a = agents[i];
        if (!a.alive || a.atGoal) continue;

        int cx = (int)(a.x * GRID); cx = std::clamp(cx, 0, GRID - 1);
        int cy = (int)(a.y * GRID); cy = std::clamp(cy, 0, GRID - 1);

        // Build list of allowed neighbours
        float weights[4] = {};
        bool  valid[4] = {};

        for (int d = 0; d < 4; ++d) {
            int nx = cx + dx4[d];
            int ny = cy + dy4[d];
            if (nx < 0 || nx >= GRID || ny < 0 || ny >= GRID) continue;

            float tx = (nx + 0.5f) / GRID;
            float ty = (ny + 0.5f) / GRID;

            if (!scenario.canMoveTo(a.x, a.y, tx, ty)) continue;

            float tau = std::pow(m_tau[ny][nx] + 1e-6f, m_alpha);
            float eta = std::pow(heuristic(nx, ny, goalX, goalY), m_beta);
            weights[d] = tau * eta;
            valid[d] = true;
        }

        float total = 0.0f;
        for (int d = 0; d < 4; ++d) total += weights[d];

        if (total < 1e-9f) {
            // Completely stuck – random valid move
            for (int d = 0; d < 4; ++d) {
                if (valid[d]) weights[d] = 1.0f;
            }
            for (int d = 0; d < 4; ++d) total += weights[d];
        }

        // Roulette-wheel selection
        float r = frand() * total;
        int chosen = -1;
        float acc = 0.0f;
        for (int d = 0; d < 4; ++d) {
            acc += weights[d];
            if (r <= acc && valid[d]) { chosen = d; break; }
        }
        if (chosen == -1) {
            // fallback: pick any valid
            for (int d = 0; d < 4; ++d)
                if (valid[d]) { chosen = d; break; }
        }
        if (chosen == -1) continue;  // fully stuck, skip

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
        m_paths[i].cost += 1.0f;  // each step costs 1

        // 3. Goal check → deposit pheromones along path
        if (scenario.isAtGoal(a.x, a.y)) {
            a.atGoal = true;
            m_paths[i].reachedGoal = true;
            float delta = m_Q / (m_paths[i].cost + 1.0f);
            for (auto& cell : m_paths[i].cells)
                m_tau[cell.second][cell.first] += delta;
        }
    }
}

void AntColonyOptimization::drawOverlay(float xOffset, float widthScale,
    float yTop, float height)
{
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    float cellW = widthScale * (800.0f / GRID);   // approximate
    float cellH = height / GRID;

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
            float px = xOffset + x * cellW;
            float py = yTop + y * cellH;
            dl->AddRectFilled(ImVec2(px, py),
                ImVec2(px + cellW, py + cellH), col);
        }
    }
}