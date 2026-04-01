#include "PathPlanning.h"
#include "imgui.h"
#include <stack>
#include <cstdlib>

PathPlanning::PathPlanning(int width, int height)
    : m_width(width), m_height(height), m_finished(false)
{
}

const char* PathPlanning::getName() const {
    return "Path Planning (Maze)";
}

int PathPlanning::index(int x, int y) const {
    return x + y * m_width;
}

void PathPlanning::reset(std::vector<Agent>& agents) {
    m_finished = false;
    m_cells.clear();
    m_cells.resize(m_width * m_height);

    generateMaze();

    m_cells[index(0, 0)].walls[3] = false;
    m_cells[index(m_width - 1, m_height - 1)].walls[1] = false;

    for (auto& a : agents) {
        a.x = 0.0f;
        a.y = 0.0f;
        a.vx = 0.0f;
        a.vy = 0.0f;
    }
}

void PathPlanning::generateMaze() {
    std::stack<int> stack;

    int current = 0;
    m_cells[current].visited = true;
    stack.push(current);

    while (!stack.empty()) {
        int c = stack.top();
        int x = c % m_width;
        int y = c / m_width;

        std::vector<int> neighbors;

        // top
        if (y > 0 && !m_cells[index(x, y - 1)].visited)
            neighbors.push_back(0);
        // right
        if (x < m_width - 1 && !m_cells[index(x + 1, y)].visited)
            neighbors.push_back(1);
        // bottom
        if (y < m_height - 1 && !m_cells[index(x, y + 1)].visited)
            neighbors.push_back(2);
        // left
        if (x > 0 && !m_cells[index(x - 1, y)].visited)
            neighbors.push_back(3);

        if (!neighbors.empty()) {
            int dir = neighbors[rand() % neighbors.size()];
            int next;

            switch (dir) {
            case 0: next = index(x, y - 1); break;
            case 1: next = index(x + 1, y); break;
            case 2: next = index(x, y + 1); break;
            case 3: next = index(x - 1, y); break;
            }

            // Remove walls
            m_cells[c].walls[dir] = false;
            m_cells[next].walls[(dir + 2) % 4] = false;

            m_cells[next].visited = true;
            stack.push(next);
        }
        else {
            stack.pop();
        }
    }
}

void PathPlanning::update(float /*dt*/, std::vector<Agent>& agents) {
    // Maze itself is static
}

bool PathPlanning::isFinished() const {
    return m_finished;
}

void PathPlanning::draw(const std::vector<Agent>& agents) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

	ImGuiIO& io = ImGui::GetIO();
	float windowHeight = io.DisplaySize.y;

    float guiHeight = windowHeight / 3.0f;
    float avaliableHeight = windowHeight - guiHeight;

    float mazeHeight = m_height * 20.0f;
    float yOffset = guiHeight + (avaliableHeight - mazeHeight) / 2.0f;

    ImVec2 origin(20, yOffset);
    float cellSize = 20.0f;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const Cell& c = m_cells[index(x, y)];
            ImVec2 p(x * cellSize + origin.x, y * cellSize + origin.y);

            if (c.walls[0])
                drawList->AddLine(p, ImVec2(p.x + cellSize, p.y), IM_COL32(255, 255, 255, 255));
            if (c.walls[1])
                drawList->AddLine(ImVec2(p.x + cellSize, p.y), ImVec2(p.x + cellSize, p.y + cellSize), IM_COL32(255, 255, 255, 255));
            if (c.walls[2])
                drawList->AddLine(ImVec2(p.x, p.y + cellSize), ImVec2(p.x + cellSize, p.y + cellSize), IM_COL32(255, 255, 255, 255));
            if (c.walls[3])
                drawList->AddLine(p, ImVec2(p.x, p.y + cellSize), IM_COL32(255, 255, 255, 255));
        }
    }

    ImVec2 startTL = origin;
    ImVec2 startBR = ImVec2(startTL.x + cellSize, startTL.y + cellSize);
    drawList->AddRectFilled(startTL, startBR, IM_COL32(0, 255, 0, 255));

    ImVec2 endTL = ImVec2(origin.x + (m_width - 1) * cellSize, origin.y + (m_height - 1) * cellSize);
    ImVec2 endBR = ImVec2(endTL.x + cellSize, endTL.y + cellSize);
    drawList->AddRectFilled(endTL, endBR, IM_COL32(255, 0, 0, 255));

    float mazeWidth = m_width * cellSize;
    for (const auto& a : agents) {
        ImVec2 p(
            origin.x + a.x * mazeWidth,
            origin.y + a.y * mazeHeight  // mazeHeight already declared above
        );
        drawList->AddCircleFilled(p, 3.0f, IM_COL32(0, 200, 255, 255));
    }
}
