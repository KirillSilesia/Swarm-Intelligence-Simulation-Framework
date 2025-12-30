#include "imgui.h"
#include "implot.h"

void renderGUI() {
    ImGui::Begin("Simulation Control");

    static int agentCount = 50;
    ImGui::SliderInt("Agents", &agentCount, 10, 200);

    static int algo = 0;
    const char* algos[] = { "ACO", "PSO", "ABC" };
    ImGui::Combo("Algorithm", &algo, algos, IM_ARRAYSIZE(algos));

    static int scen = 0;
    const char* scenes[] = { "Maze", "Exploration", "Target Search" };
    ImGui::Combo("Scenario", &scen, scenes, IM_ARRAYSIZE(scenes));

    if (ImGui::Button("Run Simulation")) {
    }

    ImGui::End();
}
