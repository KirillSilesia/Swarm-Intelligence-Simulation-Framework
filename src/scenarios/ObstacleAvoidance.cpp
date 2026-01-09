#include "ObstacleAvoidance.h"
#include "imgui.h"
#include <cmath>
#include <cstdlib>

ObstacleAvoidance::ObstacleAvoidance(int width, int height)
    : m_width(width), m_height(height), m_finished(false) {
    reset();
}

const char* ObstacleAvoidance::getName() const {
    return "Obstacle Avoidance";
}

void ObstacleAvoidance::reset() {
    m_finished = false;
    m_obstacles.clear();

    ImGuiIO& io = ImGui::GetIO();
    float windowHeight = io.DisplaySize.y;

    m_corridorWidth = 600.0f;
    m_corridorHeight = 400.0f;

    float guiHeight = windowHeight / 3.0f;
    float availableHeight = windowHeight - guiHeight;

    m_corridorTopLeft = Vec2(
        50.0f,
        guiHeight + (availableHeight - m_corridorHeight) / 2.0f
    );
    m_corridorBottomRight = Vec2(
        m_corridorTopLeft.x + m_corridorWidth,
        m_corridorTopLeft.y + m_corridorHeight
    );

    m_gapSize = 60.0f;
    m_entranceY = m_corridorTopLeft.y + m_corridorHeight / 2.0f;
    m_exitY = m_entranceY;

    generateObstacles();
}

void ObstacleAvoidance::generateObstacles() {
    int numObstacles = 10 + rand() % 6;

    for (int i = 0; i < numObstacles; ++i) {
        Obstacle obs;

        obs.position = Vec2(
            m_corridorBottomRight.x + 50.0f + (rand() % 200),
            m_corridorTopLeft.y + 30.0f + (rand() % (int)(m_corridorHeight - 60.0f))
        );

        obs.velocity = Vec2(
            -(30.0f + (rand() % 70)),
            (rand() % 40) - 20.0f 
        );

        obs.size = 15.0f + (rand() % 35);

        int shapeType = rand() % 3;
        switch (shapeType) {
        case 0: obs.shape = ObstacleShape::Circle; break;
        case 1: obs.shape = ObstacleShape::Rectangle; break;
        case 2: obs.shape = ObstacleShape::Triangle; break;
        }

        obs.rotation = (rand() % 360) * 3.14159f / 180.0f;
        obs.rotationSpeed = ((rand() % 100) - 50) / 50.0f;

        m_obstacles.push_back(obs);
    }
}

bool ObstacleAvoidance::isInsideCorridor(const Vec2& pos) const {
    if (pos.x < m_corridorTopLeft.x || pos.x > m_corridorBottomRight.x) {
        return false;
    }
    if (pos.y < m_corridorTopLeft.y || pos.y > m_corridorBottomRight.y) {
        return false;
    }

    if (pos.x < m_corridorTopLeft.x + 10.0f) {
        if (pos.y < m_exitY - m_gapSize / 2.0f || pos.y > m_exitY + m_gapSize / 2.0f) {
            return false;
        }
    }

    if (pos.x > m_corridorBottomRight.x - 10.0f) {
        if (pos.y < m_entranceY - m_gapSize / 2.0f || pos.y > m_entranceY + m_gapSize / 2.0f) {
            return false;
        }
    }

    return true;
}

void ObstacleAvoidance::update(float deltaTime) {
    for (auto& obs : m_obstacles) {
        obs.position = obs.position + obs.velocity * deltaTime;

        obs.rotation += obs.rotationSpeed * deltaTime;

        if (obs.position.y < m_corridorTopLeft.y + obs.size) {
            obs.position.y = m_corridorTopLeft.y + obs.size;
            obs.velocity.y = -obs.velocity.y;
        }
        if (obs.position.y > m_corridorBottomRight.y - obs.size) {
            obs.position.y = m_corridorBottomRight.y - obs.size;
            obs.velocity.y = -obs.velocity.y;
        }

        if (obs.position.x < m_corridorTopLeft.x - obs.size - 100.0f) {
            obs.position.x = m_corridorBottomRight.x + obs.size + 50.0f + (rand() % 100);
            obs.position.y = m_corridorTopLeft.y + 30.0f + (rand() % (int)(m_corridorHeight - 60.0f));
            obs.velocity.x = -(30.0f + (rand() % 70));
            obs.velocity.y = (rand() % 40) - 20.0f;
        }
    }
}

bool ObstacleAvoidance::isFinished() const {
    return m_finished;
}

void ObstacleAvoidance::draw() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    drawList->PushClipRect(
        ImVec2(m_corridorTopLeft.x, m_corridorTopLeft.y),
        ImVec2(m_corridorBottomRight.x, m_corridorBottomRight.y),
        true
    );

    for (const auto& obs : m_obstacles) {
        ImU32 obstacleColor = IM_COL32(255, 165, 0, 255);
        ImVec2 pos(obs.position.x, obs.position.y);

        switch (obs.shape) {
        case ObstacleShape::Circle:
            drawList->AddCircleFilled(pos, obs.size, obstacleColor);
            drawList->AddCircle(pos, obs.size, IM_COL32(255, 255, 255, 255), 0, 2.0f);
            break;

        case ObstacleShape::Rectangle: {
            float halfSize = obs.size;
            ImVec2 corners[4];
            float c = std::cos(obs.rotation);
            float s = std::sin(obs.rotation);

            corners[0] = ImVec2(pos.x + (-halfSize * c - -halfSize * s),
                pos.y + (-halfSize * s + -halfSize * c));
            corners[1] = ImVec2(pos.x + (halfSize * c - -halfSize * s),
                pos.y + (halfSize * s + -halfSize * c));
            corners[2] = ImVec2(pos.x + (halfSize * c - halfSize * s),
                pos.y + (halfSize * s + halfSize * c));
            corners[3] = ImVec2(pos.x + (-halfSize * c - halfSize * s),
                pos.y + (-halfSize * s + halfSize * c));

            drawList->AddQuadFilled(corners[0], corners[1], corners[2], corners[3], obstacleColor);
            drawList->AddQuad(corners[0], corners[1], corners[2], corners[3],
                IM_COL32(255, 255, 255, 255), 2.0f);
            break;
        }

        case ObstacleShape::Triangle: {
            float c = std::cos(obs.rotation);
            float s = std::sin(obs.rotation);
            float r = obs.size;

            ImVec2 p1(pos.x + r * c, pos.y + r * s);
            ImVec2 p2(pos.x + r * std::cos(obs.rotation + 2.0944f),
                pos.y + r * std::sin(obs.rotation + 2.0944f));
            ImVec2 p3(pos.x + r * std::cos(obs.rotation - 2.0944f),
                pos.y + r * std::sin(obs.rotation - 2.0944f));

            drawList->AddTriangleFilled(p1, p2, p3, obstacleColor);
            drawList->AddTriangle(p1, p2, p3, IM_COL32(255, 255, 255, 255), 2.0f);
            break;
        }
        }
    }

    drawList->PopClipRect();

    ImU32 wallColor = IM_COL32(150, 150, 150, 255);
    float wallThickness = 3.0f;

    drawList->AddLine(
        ImVec2(m_corridorTopLeft.x, m_corridorTopLeft.y),
        ImVec2(m_corridorBottomRight.x, m_corridorTopLeft.y),
        wallColor, wallThickness
    );

    drawList->AddLine(
        ImVec2(m_corridorTopLeft.x, m_corridorBottomRight.y),
        ImVec2(m_corridorBottomRight.x, m_corridorBottomRight.y),
        wallColor, wallThickness
    );

    drawList->AddLine(
        ImVec2(m_corridorTopLeft.x, m_corridorTopLeft.y),
        ImVec2(m_corridorTopLeft.x, m_exitY - m_gapSize / 2.0f),
        wallColor, wallThickness
    );
    drawList->AddLine(
        ImVec2(m_corridorTopLeft.x, m_exitY + m_gapSize / 2.0f),
        ImVec2(m_corridorTopLeft.x, m_corridorBottomRight.y),
        wallColor, wallThickness
    );

    drawList->AddLine(
        ImVec2(m_corridorBottomRight.x, m_corridorTopLeft.y),
        ImVec2(m_corridorBottomRight.x, m_entranceY - m_gapSize / 2.0f),
        wallColor, wallThickness
    );
    drawList->AddLine(
        ImVec2(m_corridorBottomRight.x, m_entranceY + m_gapSize / 2.0f),
        ImVec2(m_corridorBottomRight.x, m_corridorBottomRight.y),
        wallColor, wallThickness
    );

    ImU32 entranceColor = IM_COL32(0, 255, 0, 255);
    ImU32 exitColor = IM_COL32(255, 0, 0, 255);
    float markerSize = 12.0f;

    drawList->AddRectFilled(
        ImVec2(m_corridorBottomRight.x - markerSize / 2, m_entranceY - markerSize / 2),
        ImVec2(m_corridorBottomRight.x + markerSize / 2, m_entranceY + markerSize / 2),
        entranceColor
    );

    drawList->AddRectFilled(
        ImVec2(m_corridorTopLeft.x - markerSize / 2, m_exitY - markerSize / 2),
        ImVec2(m_corridorTopLeft.x + markerSize / 2, m_exitY + markerSize / 2),
        exitColor
    );
}