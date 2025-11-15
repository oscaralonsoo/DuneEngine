#pragma once

#include "Module.h"
#include "SDL3/SDL.h"

#define NUM_MOUSE_BUTTONS 5

enum EventWindow
{
    WE_QUIT = 0,
    WE_HIDE = 1,
    WE_SHOW = 2,
    WE_COUNT
};

enum KeyState
{
    KEY_IDLE = 0,
    KEY_DOWN,
    KEY_REPEAT,
    KEY_UP
};

class ModuleInput : public Module
{

public:
    ModuleInput();

    // Destructor
    virtual ~ModuleInput();

    // Called before render is available
    bool Awake();

    // Called before the first frame
    bool Start();

    // Called each loop iteration
    bool PreUpdate();

    // Called before quitting
    bool CleanUp();

    // Check key states (includes mouse and joy buttons)
    KeyState GetKey(int id) const
    {
        return keyboard[id];
    }

    KeyState GetMouseButtonDown(int id) const
    {
        return mouseButtons[id - 1];
    }

    // Check if a certain window event happened
    bool GetWindowEvent(EventWindow ev) const;

    // Get mouse / axis position
    SDL_Point GetMousePosition() const;
    SDL_Point GetMouseMotion() const;

private:
    bool windowEvents[WE_COUNT];
    KeyState *keyboard;
    KeyState mouseButtons[NUM_MOUSE_BUTTONS];
    int mouseMotionX;
    int mouseMotionY;
    int mouseX;
    int mouseY;
};