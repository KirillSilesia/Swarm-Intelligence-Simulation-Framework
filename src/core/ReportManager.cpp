#include "ReportManager.h"
#include "XlsxWriter.h"
#include "imgui.h"
#include "implot.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>


std::string ReportManager::fmtTime(float t) {
    char buf[32];
    if (t < 0) snprintf(buf, sizeof(buf), "N/A");
    else        snprintf(buf, sizeof(buf), "%.2f s", t);
    return buf;
}

static void buildTimeArray(const SimulationResult& r,
    std::vector<float>& times,
    std::vector<float>& fitness,
    std::vector<float>& alive,
    std::vector<float>& speed,
    std::vector<float>& divers)
{
    times.clear(); fitness.clear(); alive.clear(); speed.clear(); divers.clear();
    for (const auto& fr : r.frames) {
        times.push_back(fr.time);
        fitness.push_back(fr.bestFitness);
        alive.push_back((float)fr.aliveAgents);
        speed.push_back(fr.avgSpeed);
        divers.push_back(fr.diversity);
    }
}


void ReportManager::setResults(const SimulationResult& r1,
    const SimulationResult* r2,
    bool dualMode)
{
    m_r1 = r1;
    if (r2) m_r2 = *r2;
    m_dualMode = dualMode;
}

void ReportManager::drawReportWindows() {
    if (!m_open) return;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.05f,
        io.DisplaySize.y * 0.08f),
        ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.90f,
        io.DisplaySize.y * 0.88f),
        ImGuiCond_Appearing);

    ImGui::Begin("Simulation Reports", &m_open,
        ImGuiWindowFlags_NoCollapse);

    float bw = 180.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - bw - 8.0f);
    if (ImGui::Button("Export to Excel (.xlsx)", ImVec2(bw, 0))) {
        std::string err = exportXlsx("SwarmSim_Report.xlsx");
        if (err.empty())
            ImGui::OpenPopup("export_ok");
        else
            ImGui::OpenPopup("export_err");
    }
    if (ImGui::BeginPopupModal("export_ok", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Saved: SwarmSim_Report.xlsx");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("export_err", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Export failed.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (ImGui::BeginTabBar("ReportTabs")) {

        const char* suf1 = m_dualMode ? " [Sim 1]" : "";
        const char* suf2 = " [Sim 2]";

        if (ImGui::BeginTabItem("Optimisation")) {
            if (ImGui::BeginTabBar("OptSub")) {
                if (ImGui::BeginTabItem(m_dualMode ? "Sim 1##opt" : "Sim 1")) {
                    drawOptimisationTab(m_r1, suf1);
                    ImGui::EndTabItem();
                }
                if (m_dualMode && ImGui::BeginTabItem("Sim 2##opt")) {
                    drawOptimisationTab(m_r2, suf2);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Swarm Behaviour")) {
            if (ImGui::BeginTabBar("SBSub")) {
                if (ImGui::BeginTabItem(m_dualMode ? "Sim 1##sb" : "Sim 1")) {
                    drawSwarmBehaviourTab(m_r1, suf1);
                    ImGui::EndTabItem();
                }
                if (m_dualMode && ImGui::BeginTabItem("Sim 2##sb")) {
                    drawSwarmBehaviourTab(m_r2, suf2);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Environment Coverage")) {
            if (ImGui::BeginTabBar("ECsub")) {
                if (ImGui::BeginTabItem(m_dualMode ? "Sim 1##ec" : "Sim 1")) {
                    drawEnvCoverageTab(m_r1, suf1);
                    ImGui::EndTabItem();
                }
                if (m_dualMode && ImGui::BeginTabItem("Sim 2##ec")) {
                    drawEnvCoverageTab(m_r2, suf2);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }

        if (m_dualMode && ImGui::BeginTabItem("Comparison")) {
            drawComparisonTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}


void ReportManager::drawOptimisationTab(const SimulationResult& r,
    const char* /*suffix*/)
{
    ImGui::Columns(2, "optcols", false);

    ImGui::TextColored(ImVec4(1, 0.85f, 0.3f, 1), "Summary");
    ImGui::Separator();
    auto row = [](const char* k, const char* v) {
        ImGui::TextUnformatted(k);
        ImGui::NextColumn();
        ImGui::TextUnformatted(v);
        ImGui::NextColumn();
        };
    ImGui::Columns(2, "sumtbl", true);
    char buf[128];
    row("Algorithm", r.algorithmName.c_str());
    row("Scenario", r.scenarioName.c_str());
    snprintf(buf, sizeof(buf), "%d", r.agentCount);
    row("Agents", buf);
    row("Completed", r.completed ? "Yes" : "No");
    row("Completion time", fmtTime(r.completionTime).c_str());
    snprintf(buf, sizeof(buf), "%d / %d", r.agentsAtGoal, r.agentCount);
    row("Agents at goal", buf);

    float bestF = 1e9f;
    for (const auto& fr : r.frames)
        if (fr.bestFitness < bestF) bestF = fr.bestFitness;
    snprintf(buf, sizeof(buf), "%.4f", bestF < 1e8f ? bestF : 0.0f);
    row("Best fitness", buf);
    ImGui::Columns(1);

    ImGui::NextColumn();

    std::vector<float> ts, fs, al, sp, di;
    buildTimeArray(r, ts, fs, al, sp, di);

    if (!ts.empty() && ImPlot::BeginPlot("Convergence", ImVec2(-1, 260))) {
        ImPlot::SetupAxes("Time (s)", "Best Fitness");
        ImPlot::PlotLine("Best Fitness",
            ts.data(), fs.data(), (int)ts.size());
        ImPlot::EndPlot();
    }

    ImGui::Columns(1);
}


void ReportManager::drawSwarmBehaviourTab(const SimulationResult& r,
    const char* /*suffix*/)
{
    std::vector<float> ts, fs, al, sp, di;
    buildTimeArray(r, ts, fs, al, sp, di);

    if (ts.empty()) { ImGui::TextDisabled("No data collected."); return; }

    float h = 200.0f;

    if (ImPlot::BeginPlot("Alive Agents Over Time", ImVec2(-1, h))) {
        ImPlot::SetupAxes("Time (s)", "Count");
        ImPlot::PlotLine("Alive", ts.data(), al.data(), (int)ts.size());
        ImPlot::EndPlot();
    }
    ImGui::Spacing();
    if (ImPlot::BeginPlot("Average Speed Over Time", ImVec2(-1, h))) {
        ImPlot::SetupAxes("Time (s)", "Speed");
        ImPlot::PlotLine("Avg Speed", ts.data(), sp.data(), (int)ts.size());
        ImPlot::EndPlot();
    }
    ImGui::Spacing();
    if (ImPlot::BeginPlot("Swarm Diversity (Position Std Dev)", ImVec2(-1, h))) {
        ImPlot::SetupAxes("Time (s)", "Std Dev");
        ImPlot::PlotLine("Diversity", ts.data(), di.data(), (int)ts.size());
        ImPlot::EndPlot();
    }
}


void ReportManager::drawEnvCoverageTab(const SimulationResult& r,
    const char* /*suffix*/)
{
    ImGui::TextColored(ImVec4(1, 0.85f, 0.3f, 1), "Coverage Summary");
    ImGui::Separator();
    ImGui::Spacing();

    char buf[128];

    if (r.highPriorityFound + r.mediumPriorityFound + r.lowPriorityFound > 0) {
        ImGui::TextUnformatted("Reconnaissance Object Coverage:");
        ImGui::Spacing();

        static const char* lbls[3] = { "High (P3)","Medium (P2)","Low (P1)" };
        static const int totals[3] = { 2, 3, 4 };
        int found[3] = { r.highPriorityFound, r.mediumPriorityFound, r.lowPriorityFound };

        if (ImPlot::BeginPlot("Objects Found by Priority", ImVec2(400, 200))) {
            ImPlot::SetupAxes("Priority", "Found");
            static double xs[3] = { 1,2,3 };
            double ys[3] = { (double)found[0],(double)found[1],(double)found[2] };
            double yt[3] = { (double)totals[0],(double)totals[1],(double)totals[2] };
            ImPlot::PlotBars("Found", xs, ys, 3, 0.35);
            ImPlot::PlotBars("Total", xs, yt, 3, 0.35);
            ImPlot::EndPlot();
        }
        ImGui::Spacing();
        for (int i = 0; i < 3; ++i) {
            snprintf(buf, sizeof(buf), "%s: %d / %d (%.0f%%)",
                lbls[i], found[i], totals[i],
                totals[i] > 0 ? found[i] * 100.0f / totals[i] : 0.0f);
            ImGui::Bullet(); ImGui::SameLine(); ImGui::TextUnformatted(buf);
        }
    }
    else {
        snprintf(buf, sizeof(buf), "Agents reached goal: %d / %d",
            r.agentsAtGoal, r.agentCount);
        ImGui::TextUnformatted(buf);

        snprintf(buf, sizeof(buf), "Survivors: %d / %d",
            r.finalAliveAgents, r.agentCount);
        ImGui::TextUnformatted(buf);

        snprintf(buf, sizeof(buf), "Scenario completed: %s",
            r.completed ? "Yes" : "No");
        ImGui::TextUnformatted(buf);

        std::vector<float> ts, fs, al, sp, di;
        buildTimeArray(r, ts, fs, al, sp, di);
        if (!ts.empty()) {
            std::vector<float> ag;
            for (const auto& fr : r.frames) ag.push_back((float)fr.atGoalAgents);
            if (ImPlot::BeginPlot("Agents Reaching Goal", ImVec2(-1, 220))) {
                ImPlot::SetupAxes("Time (s)", "Count");
                ImPlot::PlotLine("At Goal", ts.data(), ag.data(), (int)ag.size());
                ImPlot::EndPlot();
            }
        }
    }
}


void ReportManager::drawComparisonTab() {
    auto metric = [](const char* label,
        const std::string& v1, const std::string& v2)
        {
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
            ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(v1.c_str());
            ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(v2.c_str());
        };

    char b1[64], b2[64];

    ImGui::TextColored(ImVec4(1, 0.85f, 0.3f, 1), "Algorithm Comparison");
    ImGui::Separator();

    if (ImGui::BeginTable("cmptbl", 3,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Metric");
        ImGui::TableSetupColumn(m_r1.algorithmName.c_str());
        ImGui::TableSetupColumn(m_r2.algorithmName.c_str());
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        metric("Scenario",
            m_r1.scenarioName, m_r2.scenarioName);

        ImGui::TableNextRow();
        metric("Completed",
            m_r1.completed ? "Yes" : "No",
            m_r2.completed ? "Yes" : "No");

        ImGui::TableNextRow();
        snprintf(b1, sizeof(b1), "%.2f s", m_r1.completionTime);
        snprintf(b2, sizeof(b2), "%.2f s", m_r2.completionTime);
        metric("Completion Time", b1, b2);

        ImGui::TableNextRow();
        snprintf(b1, sizeof(b1), "%d / %d", m_r1.agentsAtGoal, m_r1.agentCount);
        snprintf(b2, sizeof(b2), "%d / %d", m_r2.agentsAtGoal, m_r2.agentCount);
        metric("Agents at Goal", b1, b2);

        ImGui::TableNextRow();
        snprintf(b1, sizeof(b1), "%d", m_r1.finalAliveAgents);
        snprintf(b2, sizeof(b2), "%d", m_r2.finalAliveAgents);
        metric("Survivors", b1, b2);

        auto getBestF = [](const SimulationResult& r) {
            float bf = 1e9f;
            for (const auto& fr : r.frames)
                if (fr.bestFitness < bf) bf = fr.bestFitness;
            return bf < 1e8f ? bf : 0.0f;
            };
        ImGui::TableNextRow();
        snprintf(b1, sizeof(b1), "%.4f", getBestF(m_r1));
        snprintf(b2, sizeof(b2), "%.4f", getBestF(m_r2));
        metric("Best Fitness", b1, b2);

        ImGui::EndTable();
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
    ImGui::TextColored(ImVec4(1, 0.85f, 0.3f, 1), "Convergence Comparison");

    std::vector<float> t1, f1, a1, s1, d1, t2, f2, a2, s2, d2;
    buildTimeArray(m_r1, t1, f1, a1, s1, d1);
    buildTimeArray(m_r2, t2, f2, a2, s2, d2);

    if (!t1.empty() || !t2.empty()) {
        if (ImPlot::BeginPlot("Best Fitness Convergence", ImVec2(-1, 240))) {
            ImPlot::SetupAxes("Time (s)", "Best Fitness");
            if (!t1.empty()) ImPlot::PlotLine(m_r1.algorithmName.c_str(),
                t1.data(), f1.data(), (int)t1.size());
            if (!t2.empty()) ImPlot::PlotLine(m_r2.algorithmName.c_str(),
                t2.data(), f2.data(), (int)t2.size());
            ImPlot::EndPlot();
        }
        if (ImPlot::BeginPlot("Alive Agents", ImVec2(-1, 200))) {
            ImPlot::SetupAxes("Time (s)", "Count");
            if (!t1.empty()) ImPlot::PlotLine(m_r1.algorithmName.c_str(),
                t1.data(), a1.data(), (int)a1.size());
            if (!t2.empty()) ImPlot::PlotLine(m_r2.algorithmName.c_str(),
                t2.data(), a2.data(), (int)a2.size());
            ImPlot::EndPlot();
        }
    }
}


std::string ReportManager::exportXlsx(const std::string& path) const {
    XlsxWriter xls;

    auto toStr = [](float v, int dp = 4) -> std::string {
        char buf[32]; snprintf(buf, sizeof(buf), "%.*f", dp, (double)v);
        return buf;
        };
    auto toInt = [](int v) -> std::string {
        return std::to_string(v);
        };

    xls.addSheet("Summary");
    xls.writeRow({ "Metric", "Simulation 1",
                  m_dualMode ? "Simulation 2" : "" });
    xls.writeRow({ "Algorithm", m_r1.algorithmName,
                  m_dualMode ? m_r2.algorithmName : "" });
    xls.writeRow({ "Scenario", m_r1.scenarioName,
                  m_dualMode ? m_r2.scenarioName : "" });
    xls.writeRow({ "Agent Count", toInt(m_r1.agentCount),
                  m_dualMode ? toInt(m_r2.agentCount) : "" });
    xls.writeRow({ "Completed", m_r1.completed ? "Yes" : "No",
                  m_dualMode ? (m_r2.completed ? "Yes" : "No") : "" });
    xls.writeRow({ "Completion Time (s)",
                  toStr(m_r1.completionTime, 2),
                  m_dualMode ? toStr(m_r2.completionTime,2) : "" });
    xls.writeRow({ "Agents at Goal",
                  toInt(m_r1.agentsAtGoal),
                  m_dualMode ? toInt(m_r2.agentsAtGoal) : "" });
    xls.writeRow({ "Final Survivors",
                  toInt(m_r1.finalAliveAgents),
                  m_dualMode ? toInt(m_r2.finalAliveAgents) : "" });

    xls.addSheet("Convergence_Sim1");
    xls.writeRow({ "Time (s)", "Best Fitness", "Alive Agents",
                  "At Goal", "Avg Speed", "Diversity" });
    for (const auto& fr : m_r1.frames) {
        xls.writeRow({ toStr(fr.time,2), toStr(fr.bestFitness),
                      toInt(fr.aliveAgents), toInt(fr.atGoalAgents),
                      toStr(fr.avgSpeed),   toStr(fr.diversity) });
    }

    if (m_dualMode) {
        xls.addSheet("Convergence_Sim2");
        xls.writeRow({ "Time (s)", "Best Fitness", "Alive Agents",
                      "At Goal", "Avg Speed", "Diversity" });
        for (const auto& fr : m_r2.frames) {
            xls.writeRow({ toStr(fr.time,2), toStr(fr.bestFitness),
                          toInt(fr.aliveAgents), toInt(fr.atGoalAgents),
                          toStr(fr.avgSpeed),   toStr(fr.diversity) });
        }
    }

    bool r1Recon = m_r1.highPriorityFound + m_r1.mediumPriorityFound
        + m_r1.lowPriorityFound > 0;
    bool r2Recon = m_dualMode && (m_r2.highPriorityFound + m_r2.mediumPriorityFound
        + m_r2.lowPriorityFound > 0);
    if (r1Recon || r2Recon) {
        xls.addSheet("Reconnaissance");
        xls.writeRow({ "Priority", "Found (Sim1)", "Total",
                      m_dualMode ? "Found (Sim2)" : "" });
        xls.writeRow({ "High (P3)",
                      toInt(m_r1.highPriorityFound),   "2",
                      m_dualMode ? toInt(m_r2.highPriorityFound) : "" });
        xls.writeRow({ "Medium (P2)",
                      toInt(m_r1.mediumPriorityFound), "3",
                      m_dualMode ? toInt(m_r2.mediumPriorityFound) : "" });
        xls.writeRow({ "Low (P1)",
                      toInt(m_r1.lowPriorityFound),    "4",
                      m_dualMode ? toInt(m_r2.lowPriorityFound) : "" });
    }

    if (!xls.save(path)) return "Could not write file: " + path;
    return "";
}