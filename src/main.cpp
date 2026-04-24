#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <memory>

#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "core/Agent.h"
#include "core/Simulation.h"
#include "scenarios/Scenario.h"
#include "algorithms/SwarmAlgorithm.h"
#include "algorithms/ArtificialBeeColony.h"
#include "algorithms/ParticleSwarmOptimization.h"
#include "algorithms/AntColonyOptimization.h"
#include "algorithms/FishSchoolSearch.h"
#include "scenarios/TargetSearch.h"
#include "scenarios/Reconnaissance.h"
#include "scenarios/PathPlanning.h"
#include "scenarios/ObstacleAvoidance.h"

std::shared_ptr<Scenario> currentScenario = nullptr;
std::shared_ptr<Scenario> currentScenario2 = nullptr;
float dtMultiplier = 1.0f;
bool dualMode = false;

std::shared_ptr<SwarmAlgorithm> makeAlgorithm(int choice) {
    switch (choice) {
    case 0: return std::make_shared<AntColonyOptimization>();
    case 1: return std::make_shared<ParticleSwarmOptimization>();
    case 2: return std::make_shared<ArtificialBeeColony>();
    case 3: return std::make_shared<FishSchoolSearch>();
    }
    return nullptr;
}

std::shared_ptr<Scenario> makeScenario(int choice) {
    switch (choice) {
    case 0: return std::make_shared<PathPlanning>(20, 20);
    case 1: return std::make_shared<Reconnaissance>(ImVec2(400.0f, 400.0f));
    case 2: return std::make_shared<ObstacleAvoidance>(800, 600);
    }
    return nullptr;
}

void renderGUI(Simulation& sim, Simulation& sim2,
    int& agentCount, int& algoChoice, int& scenChoice,
    int& algoChoice2, int& scenChoice2) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y / 3.0f), ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::Begin("Simulation Control", nullptr, flags);

    float checkboxWidth = 120.0f;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - checkboxWidth);
    ImGui::SetCursorPosY(8.0f);
    ImGui::Checkbox("Dual Mode", &dualMode);

    ImGui::SetCursorPosY(8.0f);
    ImGui::SetCursorPosX(8.0f);

    const char* algos[] = { "ACO", "PSO", "ABC", "FSS" };
    const char* scenes[] = { "Path Planning", "Reconnaissance", "Obstacle Avoidance" };

    ImGui::SliderFloat("Speed", &dtMultiplier, 0.1f, 10.0f, "%.1fx");
    ImGui::SliderInt("Agents", &agentCount, 10, 200);

    if (dualMode) {
        ImGui::Columns(2, "dual_columns", true);

        ImGui::Text("--- Simulation 1 ---");
        ImGui::Combo("Algorithm##1", &algoChoice, algos, IM_ARRAYSIZE(algos));
        ImGui::Combo("Scenario##1", &scenChoice, scenes, IM_ARRAYSIZE(scenes));

        ImGui::NextColumn();

        ImGui::Text("--- Simulation 2 ---");
        ImGui::Combo("Algorithm##2", &algoChoice2, algos, IM_ARRAYSIZE(algos));
        ImGui::Combo("Scenario##2", &scenChoice2, scenes, IM_ARRAYSIZE(scenes));

        ImGui::Columns(1);
    }
    else {
        ImGui::Combo("Algorithm", &algoChoice, algos, IM_ARRAYSIZE(algos));
        ImGui::Combo("Scenario", &scenChoice, scenes, IM_ARRAYSIZE(scenes));
    }

    if (ImGui::Button("Run Simulation")) {
        auto algo = makeAlgorithm(algoChoice);
        auto scen = makeScenario(scenChoice);
        if (algo && scen) {
            currentScenario = scen;
            sim.start(algo, scen, agentCount);
        }

        if (dualMode) {
            auto algo2 = makeAlgorithm(algoChoice2);
            auto scen2 = makeScenario(scenChoice2);
            if (algo2 && scen2) {
                currentScenario2 = scen2;
                sim2.start(algo2, scen2, agentCount);
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Swarm Simulation", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    Simulation sim, sim2;
    int agentCount = 50;
    int algoChoice = 0, scenChoice = 0;
    int algoChoice2 = 0, scenChoice2 = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderGUI(sim, sim2, agentCount, algoChoice, scenChoice, algoChoice2, scenChoice2);

        float dt = 0.016f * dtMultiplier;

        if (dualMode) {
            ImGuiIO& io = ImGui::GetIO();
            float halfWidth = io.DisplaySize.x / 2.0f;

            if (sim.isRunning()) sim.update(dt);
            if (currentScenario)
                currentScenario->draw(sim.getAgents(), 0.0f, 1.0f);

            if (sim2.isRunning()) sim2.update(dt);
            if (currentScenario2)
                currentScenario2->draw(sim2.getAgents(), halfWidth, 1.0f);
        }
        else {
            if (sim.isRunning()) sim.update(dt);
            if (currentScenario)
                currentScenario->draw(sim.getAgents(), 0.0f, 1.0f);
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}