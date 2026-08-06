#include <memory>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include "imgui.h"
#include "implot.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "core/Agent.h"
#include "core/Simulation.h"
#include "core/ReportManager.h"

#include "algorithms/AntColonyOptimization.h"
#include "algorithms/ParticleSwarmOptimization.h"
#include "algorithms/ArtificialBeeColony.h"
#include "algorithms/FishSchoolSearch.h"

#include "scenarios/PathPlanning.h"
#include "scenarios/Reconnaissance.h"
#include "scenarios/ObstacleAvoidance.h"

static float        g_dtMult = 1.0f;
static bool         g_dual = false;
static int          g_agents = 50;
static int          g_algo1 = 0, g_scen1 = 0;
static int          g_algo2 = 0, g_scen2 = 1;

static Simulation   g_sim1, g_sim2;
static ReportManager g_reports;

static std::shared_ptr<Scenario> g_scenario1, g_scenario2;

static std::shared_ptr<SwarmAlgorithm> makeAlgo(int idx) {
    switch (idx) {
    case 0: return std::make_shared<AntColonyOptimization>();
    case 1: return std::make_shared<ParticleSwarmOptimization>();
    case 2: return std::make_shared<ArtificialBeeColony>();
    case 3: return std::make_shared<FishSchoolSearch>();
    }
    return nullptr;
}

static std::shared_ptr<Scenario> makeScenario(int idx) {
    switch (idx) {
    case 0: return std::make_shared<PathPlanning>(20, 20);
    case 1: return std::make_shared<Reconnaissance>(ImVec2(400.0f, 400.0f));
    case 2: return std::make_shared<ObstacleAvoidance>(800, 400);
    }
    return nullptr;
}

static void renderControlPanel()
{
    const char* algos[] = { "ACO", "PSO", "ABC", "FSS" };
    const char* scens[] = { "Maze", "Reconnaissance", "Obstacle Avoidance" };

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(vp->Size.x, vp->Size.y / 3.0f),
        ImGuiCond_Always);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.16f, 1.0f));
    ImGui::Begin("Control", nullptr, flags);

    ImGui::SetCursorPosY(8.0f);

    float rightEdge = ImGui::GetWindowWidth() - 130.0f;
    ImGui::SetCursorPosX(rightEdge);
    ImGui::Checkbox("Dual Mode", &g_dual);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY());

    ImGui::SetCursorPos(ImVec2(8.0f, 8.0f));
    ImGui::SliderFloat("Speed", &g_dtMult, 0.1f, 10.0f, "%.1fx");
    ImGui::SliderInt("Agents", &g_agents, 10, 200);

    ImGui::Separator();

    if (g_dual) {
        ImGui::Columns(2, "dual_cols", true);
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Simulation 1");
        ImGui::Combo("Algorithm##1", &g_algo1, algos, 4);
        ImGui::Combo("Scenario##1", &g_scen1, scens, 3);
        ImGui::NextColumn();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Simulation 2");
        ImGui::Combo("Algorithm##2", &g_algo2, algos, 4);
        ImGui::Combo("Scenario##2", &g_scen2, scens, 3);
        ImGui::Columns(1);
    }
    else {
        ImGui::Combo("Algorithm", &g_algo1, algos, 4);
        ImGui::Combo("Scenario", &g_scen1, scens, 3);
    }

    ImGui::Spacing();

    bool sim1Done = g_sim1.hasFinished();
    bool sim2Done = !g_dual || g_sim2.hasFinished();
    bool allDone = sim1Done && sim2Done;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.55f, 0.2f, 1.0f));
    if (ImGui::Button("Run Simulation", ImVec2(160, 0))) {
        g_scenario1 = makeScenario(g_scen1);
        auto a1 = makeAlgo(g_algo1);
        if (a1 && g_scenario1)
            g_sim1.start(a1, g_scenario1, g_agents);

        if (g_dual) {
            g_scenario2 = makeScenario(g_scen2);
            auto a2 = makeAlgo(g_algo2);
            if (a2 && g_scenario2)
                g_sim2.start(a2, g_scenario2, g_agents);
        }
    }
    ImGui::PopStyleColor();

    bool anyRunning = g_sim1.isRunning() || (g_dual && g_sim2.isRunning());
    if (anyRunning) {
        ImGui::SameLine(0, 8);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Stop Simulation", ImVec2(160, 0))) {
            g_sim1.stop();
            if (g_dual) g_sim2.stop();
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::SameLine(0, 16);
    if (g_sim1.isRunning())
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "[Sim 1 Running]");
    else if (g_sim1.hasFinished())
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.3f, 1.0f), "[Sim 1 Finished]");
    if (g_dual) {
        ImGui::SameLine(0, 12);
        if (g_sim2.isRunning())
            ImGui::TextColored(ImVec4(0.3f, 0.6f, 1.0f, 1.0f), "[Sim 2 Running]");
        else if (g_sim2.hasFinished())
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.3f, 1.0f), "[Sim 2 Finished]");
    }

    if (allDone && (g_sim1.hasFinished())) {
        ImGui::SameLine(0, 24);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.3f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.1f, 1.0f));
        if (ImGui::Button("View Reports", ImVec2(140, 0))) {
            g_reports.setResults(g_sim1.getResult(),
                g_dual ? &g_sim2.getResult() : nullptr,
                g_dual);
            g_reports.open();
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

static void renderSim(const Simulation& sim,
    const std::shared_ptr<Scenario>& scen,
    float xOffset, float widthScale)
{
    if (!scen) return;
    scen->draw(sim.getAgents(), xOffset, widthScale);

    // draw() has just set the scenario's view transform; the overlay reads it
    // to align itself to the same on-screen rectangle.
    if (sim.getAlgorithm())
        sim.getAlgorithm()->drawOverlay(*scen);
}

int main() {
    srand((unsigned)time(nullptr));

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1400, 800,
        "Swarm Intelligence Framework",
        nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.16f, 0.96f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float dt = 0.016f * g_dtMult;
        if (g_sim1.isRunning()) g_sim1.update(dt);
        if (g_dual && g_sim2.isRunning()) g_sim2.update(dt);

        renderControlPanel();

        if (g_dual) {
            float half = io.DisplaySize.x * 0.5f;
            renderSim(g_sim1, g_scenario1, 0.0f, 0.5f);
            renderSim(g_sim2, g_scenario2, half, 0.5f);

            ImDrawList* dl = ImGui::GetBackgroundDrawList();
            float guiH = io.DisplaySize.y / 3.0f;
            dl->AddLine(ImVec2(half, guiH),
                ImVec2(half, io.DisplaySize.y),
                IM_COL32(100, 100, 100, 160), 1.5f);
        }
        else {
            renderSim(g_sim1, g_scenario1, 0.0f, 1.0f);
        }

        g_reports.drawReportWindows();

        ImGui::Render();
        int dw, dh;
        glfwGetFramebufferSize(window, &dw, &dh);
        glViewport(0, 0, dw, dh);
        glClearColor(0.07f, 0.07f, 0.10f, 1.0f);
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