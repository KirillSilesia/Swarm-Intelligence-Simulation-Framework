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

    m_corridorWidth = 600.0f;
    m_corridorHeight = 400.0f;
    m_gapSize = 60.0f;

    generateObstacles();
}

void ObstacleAvoidance::generateObstacles() {
    int numObstacles = 10 + rand() % 6;

    for (int i = 0; i < numObstacles; ++i) {
        Obstacle obs;

        obs.relativeX = 1.0f + (50.0f + (rand() % 200)) / m_corridorWidth;
        obs.relativeY = (30.0f + (rand() % (int)(m_corridorHeight - 60.0f))) / m_corridorHeight;

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

bool ObstacleAvoidance::isInsideCorridor(const Vec2& pos, const Vec2& topLeft, const Vec2& bottomRight) const {
    if (pos.x < topLeft.x || pos.x > bottomRight.x) {
        return false;
    }
    if (pos.y < topLeft.y || pos.y > bottomRight.y) {
        return false;
    }

    float entranceY = topLeft.y + m_corridorHeight / 2.0f;
    float exitY = entranceY;

    if (pos.x < topLeft.x + 10.0f) {
        if (pos.y < exitY - m_gapSize / 2.0f || pos.y > exitY + m_gapSize / 2.0f) {
            return false;
        }
    }

    if (pos.x > bottomRight.x - 10.0f) {
        if (pos.y < entranceY - m_gapSize / 2.0f || pos.y > entranceY + m_gapSize / 2.0f) {
            return false;
        }
    }

    return true;
}

void ObstacleAvoidance::update(float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();
    float windowHeight = io.DisplaySize.y;
    float guiHeight = windowHeight / 3.0f;
    float availableHeight = windowHeight - guiHeight;
    float yOffset = guiHeight + (availableHeight - m_corridorHeight) / 2.0f;

    Vec2 corridorTopLeft(20.0f, yOffset);
    Vec2 corridorBottomRight(corridorTopLeft.x + m_corridorWidth, corridorTopLeft.y + m_corridorHeight);

    for (auto& obs : m_obstacles) {
        Vec2 position(
            corridorTopLeft.x + obs.relativeX * m_corridorWidth,
            corridorTopLeft.y + obs.relativeY * m_corridorHeight
        );

        position = position + obs.velocity * deltaTime;

        obs.rotation += obs.rotationSpeed * deltaTime;

        if (position.y < corridorTopLeft.y + obs.size) {
            position.y = corridorTopLeft.y + obs.size;
            obs.velocity.y = -obs.velocity.y;
        }
        if (position.y > corridorBottomRight.y - obs.size) {
            position.y = corridorBottomRight.y - obs.size;
            obs.velocity.y = -obs.velocity.y;
        }

        if (position.x < corridorTopLeft.x - obs.size - 100.0f) {
            obs.relativeX = 1.0f + (50.0f + (rand() % 100)) / m_corridorWidth;
            obs.relativeY = (30.0f + (rand() % (int)(m_corridorHeight - 60.0f))) / m_corridorHeight;
            obs.velocity.x = -(30.0f + (rand() % 70));
            obs.velocity.y = (rand() % 40) - 20.0f;
        }
        else {
            obs.relativeX = (position.x - corridorTopLeft.x) / m_corridorWidth;
            obs.relativeY = (position.y - corridorTopLeft.y) / m_corridorHeight;
        }
    }
}

bool ObstacleAvoidance::isFinished() const {
    return m_finished;
}

void ObstacleAvoidance::draw() {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float windowHeight = io.DisplaySize.y;
    float guiHeight = windowHeight / 3.0f;
    float availableHeight = windowHeight - guiHeight;

    float yOffset = guiHeight + (availableHeight - m_corridorHeight) / 2.0f;
    Vec2 corridorTopLeft(20.0f, yOffset);
    Vec2 corridorBottomRight(corridorTopLeft.x + m_corridorWidth, corridorTopLeft.y + m_corridorHeight);

    float entranceY = corridorTopLeft.y + m_corridorHeight / 2.0f;
    float exitY = entranceY;

    drawList->PushClipRect(
        ImVec2(corridorTopLeft.x, corridorTopLeft.y),
        ImVec2(corridorBottomRight.x, corridorBottomRight.y),
        true
    );

    for (const auto& obs : m_obstacles) {
        ImU32 obstacleColor = IM_COL32(255, 165, 0, 255);

        ImVec2 pos(
            corridorTopLeft.x + obs.relativeX * m_corridorWidth,
            corridorTopLeft.y + obs.relativeY * m_corridorHeight
        );

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
        ImVec2(corridorTopLeft.x, corridorTopLeft.y),
        ImVec2(corridorBottomRight.x, corridorTopLeft.y),
        wallColor, wallThickness
    );

    drawList->AddLine(
        ImVec2(corridorTopLeft.x, corridorBottomRight.y),
        ImVec2(corridorBottomRight.x, corridorBottomRight.y),
        wallColor, wallThickness
    );

    drawList->AddLine(
        ImVec2(corridorTopLeft.x, corridorTopLeft.y),
        ImVec2(corridorTopLeft.x, exitY - m_gapSize / 2.0f),
        wallColor, wallThickness
    );
    drawList->AddLine(
        ImVec2(corridorTopLeft.x, exitY + m_gapSize / 2.0f),
        ImVec2(corridorTopLeft.x, corridorBottomRight.y),
        wallColor, wallThickness
    );

    drawList->AddLine(
        ImVec2(corridorBottomRight.x, corridorTopLeft.y),
        ImVec2(corridorBottomRight.x, entranceY - m_gapSize / 2.0f),
        wallColor, wallThickness
    );
    drawList->AddLine(
        ImVec2(corridorBottomRight.x, entranceY + m_gapSize / 2.0f),
        ImVec2(corridorBottomRight.x, corridorBottomRight.y),
        wallColor, wallThickness
    );

    ImU32 entranceColor = IM_COL32(0, 255, 0, 255);
    ImU32 exitColor = IM_COL32(255, 0, 0, 255);
    float markerSize = 12.0f;

    drawList->AddRectFilled(
        ImVec2(corridorBottomRight.x - markerSize / 2, entranceY - markerSize / 2),
        ImVec2(corridorBottomRight.x + markerSize / 2, entranceY + markerSize / 2),
        entranceColor
    );

    drawList->AddRectFilled(
        ImVec2(corridorTopLeft.x - markerSize / 2, exitY - markerSize / 2),
        ImVec2(corridorTopLeft.x + markerSize / 2, exitY + markerSize / 2),
        exitColor
    );
}