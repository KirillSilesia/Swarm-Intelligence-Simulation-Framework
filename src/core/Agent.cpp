#include <iostream>
#include "imgui.h"

struct Agent {
    ImVec2 pos;
    ImVec2 velocity;

    float bestValue;
    ImVec2 bestPos;
};