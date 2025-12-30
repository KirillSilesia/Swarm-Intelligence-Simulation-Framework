#pragma once
#include <string>

/**
 * Stores results of a single simulation run or averaged experiment.
 */
struct SimulationResult {
    std::string algorithmName;
    std::string scenarioName;

    int agentCount = 0;
    int runs = 1;

    float averageCompletionTime = 0.0f;
    float minTime = 0.0f;
    float maxTime = 0.0f;
};
