#include "GameTime.h"
#include <SDL3/SDL.h>

// ==== Static members ====
TimeState      GameTime::s_State         = TimeState::Stopped;
float          GameTime::s_TimeScale     = 1.0f;

float          GameTime::s_RealTime      = 0.0f;
float          GameTime::s_GameTime      = 0.0f;
float          GameTime::s_RealDeltaTime = 0.0f;
float          GameTime::s_GameDeltaTime = 0.0f;

bool           GameTime::s_StepRequested = false;
bool           GameTime::s_StepThisFrame = false;

std::uint64_t  GameTime::s_LastTicks     = 0;

// ==== Init / Update ====
void GameTime::Init()
{
    s_State         = TimeState::Stopped;
    s_TimeScale     = 1.0f;
    s_RealTime      = 0.0f;
    s_GameTime      = 0.0f;
    s_RealDeltaTime = 0.0f;
    s_GameDeltaTime = 0.0f;

    s_StepRequested = false;
    s_StepThisFrame = false;

    s_LastTicks = SDL_GetTicks();
}

void GameTime::Update()
{
    std::uint64_t now  = SDL_GetTicks();
    std::uint64_t diff = now - s_LastTicks;
    s_LastTicks = now;

    float realDt = static_cast<float>(diff) / 1000.0f;
    UpdateInternal(realDt);
}

void GameTime::UpdateInternal(float realDt)
{
    s_RealDeltaTime = realDt;
    s_RealTime     += realDt;

    // Consumimos la petición de step y la aplicamos SOLO en este frame
    s_StepThisFrame = s_StepRequested;
    s_StepRequested = false;

    float gameDt = 0.0f;

    switch (s_State)
    {
    case TimeState::Playing:
        gameDt = realDt * s_TimeScale;
        break;

    case TimeState::Paused:
        if (s_StepThisFrame)
            gameDt = realDt * s_TimeScale;
        break;

    case TimeState::Stopped:
        gameDt = 0.0f;
        break;
    }

    s_GameDeltaTime = gameDt;
    s_GameTime     += gameDt;
}

// ==== Getters ====
float GameTime::GetRealDeltaTime()        { return s_RealDeltaTime; }
float GameTime::GetGameDeltaTime()        { return s_GameDeltaTime; }
float GameTime::GetRealTimeSinceStartup() { return s_RealTime; }
float GameTime::GetGameTime()             { return s_GameTime; }
float GameTime::GetTimeScale()            { return s_TimeScale; }

// ==== TimeScale ====
void GameTime::SetTimeScale(float scale)
{
    if (scale < 0.0f) scale = 0.0f;
    s_TimeScale = scale;
}

// ==== Control de estado ====
void GameTime::Play()
{
    if (s_State == TimeState::Stopped)
    {
        s_GameTime      = 0.0f;
        s_GameDeltaTime = 0.0f;
    }
    s_State = TimeState::Playing;
}

void GameTime::Pause()
{
    if (s_State == TimeState::Playing)
        s_State = TimeState::Paused;
}

void GameTime::Stop()
{
    s_State = TimeState::Stopped;
    s_GameTime      = 0.0f;
    s_GameDeltaTime = 0.0f;
}

void GameTime::StepOneFrame()
{
    if (s_State == TimeState::Paused)
        s_StepRequested = true;
}

// ==== Queries ====
bool GameTime::IsPlaying()    { return s_State == TimeState::Playing; }
bool GameTime::IsPaused()     { return s_State == TimeState::Paused;  }
bool GameTime::IsStopped()    { return s_State == TimeState::Stopped; }
bool GameTime::IsStepFrame()  { return s_StepThisFrame; }
