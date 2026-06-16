#pragma once
#include "SimulationResult.h"
#include <string>

/*
 * Renders in-app report windows (ImGui + ImPlot) and exports XLSX.
 *
 * Usage:
 *   static ReportManager rm;
 *   rm.setResults(r1, r2, isDual);   // call once after both sims finish
 *   rm.drawReportWindows();           // call each frame inside ImGui frame
 *   if (ImGui::Button("Export XLSX")) rm.exportXlsx("SwarmSim_Report.xlsx");
 */
class ReportManager {
public:
    void setResults(const SimulationResult& r1,
        const SimulationResult* r2,   // nullptr in single mode
        bool dualMode);

    // Call every frame inside an ImGui::NewFrame()/EndFrame() pair.
    void drawReportWindows();

    // Write all report data to an Excel file.  Returns "" on success,
    // or an error message.
    std::string exportXlsx(const std::string& path) const;

    bool isOpen() const { return m_open; }
    void open() { m_open = true; }
    void close() { m_open = false; }

private:
    SimulationResult m_r1, m_r2;
    bool m_dualMode = false;
    bool m_open = false;

    void drawOptimisationTab(const SimulationResult& r, const char* suffix);
    void drawSwarmBehaviourTab(const SimulationResult& r, const char* suffix);
    void drawEnvCoverageTab(const SimulationResult& r, const char* suffix);
    void drawComparisonTab();

    static std::string fmtTime(float t);
};