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

// ==================== GUI ====================
void renderGUI(Simulation& sim, int& agentCount, int& algoChoice, int& scenChoice) {
	ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 windowPos = ImVec2(
        viewport->Pos.x,
        viewport->Pos.y
    );

    ImVec2 windowSize = ImVec2(
        viewport->Size.x,
        viewport->Size.y / 3.0f
    );

	ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

    ImGui::Begin("Simulation Control", nullptr, flags);

    ImGui::SliderInt("Agents", &agentCount, 10, 200);

    const char* algos[] = { "ACO", "PSO", "ABC", "FSS" };
    ImGui::Combo("Algorithm", &algoChoice, algos, IM_ARRAYSIZE(algos));

    const char* scenes[] = { "Path Planning", "Reconnaissance", "Target Search", "Obstacle Avoidance" };
    ImGui::Combo("Scenario", &scenChoice, scenes, IM_ARRAYSIZE(scenes));

    if (ImGui::Button("Run Simulation")) {
        // Choose algorithm
        std::shared_ptr<SwarmAlgorithm> algorithm;
        switch (algoChoice) {
        case 0: algorithm = std::make_shared<AntColonyOptimization>(); break;
        case 1: algorithm = std::make_shared<ParticleSwarmOptimization>(); break;
        case 2: algorithm = std::make_shared<ArtificialBeeColony>(); break;
		case 3: algorithm = std::make_shared<FishSchoolSearch>(); break;
        }

        // Choose scenario
        std::shared_ptr<Scenario> scenario;
        switch (scenChoice) {
        case 0: scenario = std::make_shared<PathPlanning>(20, 20); break;
        case 1: scenario = std::make_shared<Reconnaissance>(800, 600); break;
        case 2: scenario = std::make_shared<TargetSearch>(); break;
		case 3: scenario = std::make_shared<ObstacleAvoidance>(800, 600); break;
        }

        // Launch simulation
        if (algorithm && scenario) {
			currentScenario = scenario;
            sim.start(algorithm, scenario, agentCount);
            std::cout << "Simulation started!\n";
        }
    }

    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// ==================== Main ====================
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    // Initializing GLFW
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Swarm Simulation", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    // Initializing ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    Simulation sim;
    int agentCount = 50, algoChoice = 0, scenChoice = 0;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderGUI(sim, agentCount, algoChoice, scenChoice);

        // Updating simulation
        if (sim.isRunning()) {
            sim.update(0.016f); // dt ~16ms
        };
        if (currentScenario) {
            currentScenario->draw();
        };

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Clear
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}