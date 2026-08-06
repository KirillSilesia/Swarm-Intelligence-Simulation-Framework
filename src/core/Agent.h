#pragma once

struct Agent {
    float x = 0.0f, y = 0.0f;
    float vx = 0.0f, vy = 0.0f;  

    bool alive = true;           
    bool atGoal = false;           

    float bestX = 0.0f;
    float bestY = 0.0f;
    float bestFitness = 1e9f;

    int trial = 0;

    float weight = 1.0f;
    float prevFitness = 1e9f;
    float fitnessGain = 0.0f;
    float indivDx = 0.0f;
    float indivDy = 0.0f;

    float distTraveled = 0.0f;
    float timeAlive = 0.0f;
};