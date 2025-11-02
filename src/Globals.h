#pragma once

#include <SDL3/SDL.h>
#include <cstdio>

// ==== ENGINE GLOBAL DEFINES ====
#define ENGINE_NAME "DuneEngine"
#define ENGINE_VERSION "0.1"
#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

// ==== LOGGING ====
#define LOG_INFO(fmt, ...)   SDL_Log("[INFO]  " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   SDL_Log("[WARN]  " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  SDL_Log("[ERROR] " fmt "\n", ##__VA_ARGS__)
