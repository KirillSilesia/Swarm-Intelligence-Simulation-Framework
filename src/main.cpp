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
#include "scenarios/TargetSearchScenario.h"
#include "scenarios/ExplorationScenario.h"
#include "scenarios/MazeScenario.h"

// ==================== GUI ====================
void renderGUI(Simulation& sim, int& agentCount, int& algoChoice, int& scenChoice) {
    ImGui::Begin("Simulation Control");

    ImGui::SliderInt("Agents", &agentCount, 10, 200);

    const char* algos[] = { "ACO", "PSO", "ABC" };
    ImGui::Combo("Algorithm", &algoChoice, algos, IM_ARRAYSIZE(algos));

    const char* scenes[] = { "Maze", "Exploration", "Target Search" };
    ImGui::Combo("Scenario", &scenChoice, scenes, IM_ARRAYSIZE(scenes));

    if (ImGui::Button("Run Simulation")) {
        // Choose algorithm
        std::shared_ptr<SwarmAlgorithm> algorithm;
        switch (algoChoice) {
        case 0: algorithm = std::make_shared<AntColonyOptimization>(); break;
        case 1: algorithm = std::make_shared<ParticleSwarmOptimization>(); break;
        case 2: algorithm = std::make_shared<ArtificialBeeColony>(); break;
        }

        // Choose scenario
        std::shared_ptr<Scenario> scenario;
        switch (scenChoice) {
        case 0: scenario = std::make_shared<MazeScenario>(20, 20); break;
        case 1: scenario = std::make_shared<ExplorationScenario>(); break;
        case 2: scenario = std::make_shared<TargetSearchScenario>(); break;
        }

        // Launch simulation
        if (algorithm && scenario) {
            sim.start(algorithm, scenario, agentCount);
            std::cout << "Simulation started!\n";
        }
    }

    ImGui::End();
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

    // Clear
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}