#pragma once

class IScene {
public:
    virtual ~IScene() {}
    virtual void Init() = 0;
    virtual void Run() = 0;
    virtual void ProcessEvents() = 0;
    virtual void Update() = 0;
    virtual void Render() = 0;
};