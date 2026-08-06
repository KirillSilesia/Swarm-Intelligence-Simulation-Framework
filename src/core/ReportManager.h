#pragma once
#include "SimulationResult.h"
#include <string>

class ReportManager {
public:
    void setResults(const SimulationResult& r1,
        const SimulationResult* r2,
        bool dualMode);

    void drawReportWindows();

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