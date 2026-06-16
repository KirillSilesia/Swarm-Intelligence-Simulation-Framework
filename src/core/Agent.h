#pragma once

struct Agent {
    float x = 0.0f, y = 0.0f;          // normalized position [0..1]
    float vx = 0.0f, vy = 0.0f;        // velocity

    bool alive = true;               // ObstacleAvoidance: dies on collision
    bool atGoal = false;              // reached red square

    // PSO – personal best
    float bestX = 0.0f;
    float bestY = 0.0f;
    float bestFitness = 1e9f;

    // ABC – trial counter per food source
    int trial = 0;

    // FSS – individual weight and per-frame fitness bookkeeping
    float weight = 1.0f;
    float prevFitness = 1e9f;
    float fitnessGain = 0.0f;
    float indivDx = 0.0f;
    float indivDy = 0.0f;

    // Stats for reports
    float distTraveled = 0.0f;
    float timeAlive = 0.0f;
};