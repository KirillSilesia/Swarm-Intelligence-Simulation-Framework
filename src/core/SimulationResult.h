#pragma once
#include <string>
#include <vector>

struct ReportFrame {
    float time = 0.0f;
    float bestFitness = 1e9f;
    int   aliveAgents = 0;
    int   atGoalAgents = 0;
    float avgSpeed = 0.0f;
    float diversity = 0.0f;
};

struct SimulationResult {
    std::string algorithmName;
    std::string scenarioName;
    int   agentCount = 0;
    float completionTime = 0.0f;
    int   agentsAtGoal = 0;
    int   finalAliveAgents = 0;
    bool  completed = false;

    std::vector<ReportFrame> frames;

    int   highPriorityFound = 0;
    int   mediumPriorityFound = 0;
    int   lowPriorityFound = 0;
    float timeAllHighPriority = -1.0f;

    int   totalCollisions = 0;
};