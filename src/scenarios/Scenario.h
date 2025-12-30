#pragma once

class Scenario {
public:
    virtual ~Scenario() = default;

    virtual const char* getName() const = 0;
    virtual void reset() = 0;
    virtual void update(float deltaTime) = 0;
    virtual bool isFinished() const = 0;
};
