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


void ObstacleAvoidance::generateObstacles() {
    m_obstacles.clear();
    int n = 8 + rand() % 6;
    for (int i = 0; i < n; ++i) {
        MovingObstacle o;
        o.rx = 0.15f + frand() * 0.7f;
        o.ry = 0.1f + frand() * 0.8f;
        float speedX = -(0.03f + frand() * 0.07f);
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


float ObstacleAvoidance::evaluateFitness(float x, float y) const {
    float base = 1.0f - x;

    // Funnel toward the exit gap. The gap is only at the centre of the right
    // wall, so being off the centre line has to cost something -- otherwise
    // agents that reach the wall off-centre see no reason to slide to the gap
    // and just pile against the wall. Weighting by x^2 keeps this negligible on
    // the open left side (where they should simply advance and dodge obstacles)
    // and dominant near the wall, where alignment with the gap is what matters.
    base += x * x * std::abs(y - 0.5f) * 1.5f;

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
    float gapNorm = m_gapHalf / m_corridorH;
    if (x2 < 0.02f) {
        if (std::abs(y2 - 0.5f) > gapNorm) return false;
    }
    if (x2 > 0.98f) {
        if (std::abs(y2 - 0.5f) > gapNorm) return false;
    }

    // Refuse to step into an obstacle. This is what makes it an avoidance
    // scenario for every algorithm: a blocked move triggers each one's
    // wall-slide, so agents route around obstacles instead of driving into
    // them on momentum. A little margin over the kill radius keeps them clear.
    // (Obstacles can still drift onto a stationary agent, so collisions drop
    // sharply rather than vanishing.)
    for (const auto& o : m_obstacles) {
        float dx = x2 - o.rx, dy = y2 - o.ry;
        float block = o.normSize * 1.4f;
        if (dx * dx + dy * dy < block * block) return false;
    }
    return true;
}

bool ObstacleAvoidance::guidanceDir(float x, float y, float& dirX, float& dirY) const {
    // Steering hint toward the exit gap, for swarms (PSO) that chase the best
    // *position* found so far and would otherwise never converge on the gap.
    //
    // Base pull: push right at the agent's current height, and only draw toward
    // the centre as the right wall nears (x^2 weight). Pulling everyone onto the
    // centre line up front would make the whole swarm one target for a passing
    // obstacle, so we let them travel spread out and funnel only at the end.
    float ux = 1.0f;
    float uy = (0.5f - y) * x * x * 2.0f;

    // Avoidance: veer vertically around any obstacle that lies ahead and close.
    // Without this the rightward pull aims straight into obstacles and fights
    // each algorithm's wall-slide, leaving agents stuck grinding against them.
    for (const auto& o : m_obstacles) {
        float ox = o.rx - x, oy = o.ry - y;
        float d = std::hypot(ox, oy);
        float range = o.normSize * 3.0f + 0.07f;
        if (ox > -o.normSize && d < range) {
            float w = (range - d) / range;            // 0..1, stronger when closer
            float away = (y >= o.ry) ? 1.0f : -1.0f;  // steer to the nearer side
            uy += away * w * 2.5f;
            ux *= (1.0f - 0.6f * w);                   // ease off the throttle when close
        }
    }

    float len = std::hypot(ux, uy);
    if (len < 1e-6f) { dirX = 1.0f; dirY = 0.0f; return true; }
    dirX = ux / len;
    dirY = uy / len;
    return true;
}


void ObstacleAvoidance::reset(std::vector<Agent>& agents) {
    m_finished = false;
    m_collisions = 0;
    generateObstacles();

    float sx = getStartX(), sy = getStartY();
    for (auto& a : agents) {
        // Spread the swarm across the corridor height at the start. If they all
        // spawn on the centre line, the whole group funnels into the first
        // central obstacle and a single one can trap or wipe out everyone --
        // spreading them means each finds its own way past and only a fraction
        // ever meets any given obstacle.
        a.x = sx + (frand() - 0.5f) * 0.02f;
        a.y = sy + (frand() - 0.5f) * 0.7f;
        a.x = std::clamp(a.x, 0.0f, 1.0f);
        a.y = std::clamp(a.y, 0.05f, 0.95f);
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

    for (auto& o : m_obstacles) {
        o.rx += o.vx * dt;
        o.ry += o.vy * dt;
        o.rotation += o.rotSpeed * dt;

        if (o.ry < 0.05f) { o.ry = 0.05f; o.vy = std::abs(o.vy); }
        if (o.ry > 0.95f) { o.ry = 0.95f; o.vy = -std::abs(o.vy); }

        if (o.rx < -0.1f) {
            o.rx = 1.05f + frand() * 0.3f;
            o.ry = 0.1f + frand() * 0.8f;
        }
    }

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

    agentsAtGoal = 0;
    float gapNorm = m_gapHalf / m_corridorH;
    for (auto& a : agents) {
        if (!a.alive) continue;
        if (a.x >= 0.95f && std::abs(a.y - 0.5f) <= gapNorm)
            a.atGoal = true;
        if (a.atGoal) ++agentsAtGoal;
    }

    int alive = 0;
    for (const auto& a : agents) if (a.alive) ++alive;
    if (agentsAtGoal >= (int)(totalAgents * 0.5f) ||
        (alive == 0 && totalAgents > 0))
        m_finished = true;
}

bool ObstacleAvoidance::isFinished() const { return m_finished; }


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

    viewX = TL.x; viewY = TL.y; viewW = cW; viewH = cH;

    dl->AddRectFilled(TL, BR, IM_COL32(20, 20, 30, 220));

    dl->PushClipRect(TL, BR, true);

    for (const auto& o : m_obstacles) {
        ImVec2 p(TL.x + o.rx * cW, TL.y + o.ry * cH);
        float  sz = o.displaySize * widthScale;
        ImU32  fc = IM_COL32(220, 100, 20, 230);
        ImU32  oc = IM_COL32(255, 200, 100, 200);

        switch (o.shape) {
        case 0:
            dl->AddCircleFilled(p, sz, fc);
            dl->AddCircle(p, sz, oc, 0, 1.5f);
            break;
        case 1: {
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
        case 2: {
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

    for (const auto& a : agents) {
        if (!a.alive) continue;
        ImVec2 p(TL.x + a.x * cW, TL.y + a.y * cH);
        ImU32 col = a.atGoal ? IM_COL32(255, 220, 0, 255) : IM_COL32(60, 200, 255, 230);
        dl->AddCircleFilled(p, 3.5f, col);
    }
    dl->PopClipRect();

    ImU32 wc = IM_COL32(160, 160, 180, 255);
    float wt = 2.5f;
    dl->AddLine(TL, ImVec2(BR.x, TL.y), wc, wt);
    dl->AddLine(ImVec2(TL.x, BR.y), BR, wc, wt);
    dl->AddLine(TL, ImVec2(TL.x, midY - gapPx), wc, wt);
    dl->AddLine(ImVec2(TL.x, midY + gapPx), ImVec2(TL.x, BR.y), wc, wt);
    dl->AddLine(ImVec2(BR.x, TL.y), ImVec2(BR.x, midY - gapPx), wc, wt);
    dl->AddLine(ImVec2(BR.x, midY + gapPx), BR, wc, wt);

    float ms = 16.0f;
    dl->AddRectFilled(ImVec2(TL.x - ms * 0.5f, midY - ms * 0.5f),
        ImVec2(TL.x + ms * 0.5f, midY + ms * 0.5f),
        IM_COL32(0, 200, 80, 220));
    dl->AddText(ImVec2(TL.x - 5, midY - 6), IM_COL32(255, 255, 255, 240), "S");

    dl->AddRectFilled(ImVec2(BR.x - ms * 0.5f, midY - ms * 0.5f),
        ImVec2(BR.x + ms * 0.5f, midY + ms * 0.5f),
        IM_COL32(220, 40, 40, 220));
    dl->AddText(ImVec2(BR.x - 5, midY - 6), IM_COL32(255, 255, 255, 240), "G");

    int alive = 0;
    for (const auto& a : agents) if (a.alive) ++alive;
    char buf[128];
    snprintf(buf, sizeof(buf), "Alive: %d/%d  Goal: %d  Collisions: %d",
        alive, totalAgents, agentsAtGoal, m_collisions);
    dl->AddText(ImVec2(TL.x + 4, TL.y + 4), IM_COL32(220, 220, 220, 220), buf);
}