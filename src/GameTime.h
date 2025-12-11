#pragma once
#include <cstdint>

enum class TimeState
{
    Stopped = 0,
    Playing,
    Paused
};

class GameTime
{
public:
    // Se llama una vez cuando el engine está listo para empezar el loop
    static void Init();

    // Se llama una vez por frame, desde Engine::Update()
    static void Update();

    // ==== Getters ====
    static float GetRealDeltaTime();        // dt en segundos, tiempo real
    static float GetGameDeltaTime();        // dt en segundos, afectado por timeScale y estado
    static float GetRealTimeSinceStartup(); // tiempo real acumulado
    static float GetGameTime();             // tiempo de juego acumulado
    static float GetTimeScale();

    // ==== Control de escala de tiempo ====
    static void SetTimeScale(float scale);

    // ==== Control de estado de simulación ====
    static void Play();
    static void Pause();
    static void Stop();
    static void StepOneFrame();             // avanza un frame estando en pausa

    static bool IsPlaying();
    static bool IsPaused();
    static bool IsStopped();

    // Flag para saber si este frame es un "step" desde pause
    static bool IsStepFrame();

private:
    static void UpdateInternal(float realDt);

private:
    static TimeState s_State;

    static float s_TimeScale;

    static float s_RealTime;
    static float s_GameTime;

    static float s_RealDeltaTime;
    static float s_GameDeltaTime;

    static bool s_StepRequested;   
    static bool s_StepThisFrame;

    static bool s_StepFrame;          

    static std::uint64_t s_LastTicks; 
};
