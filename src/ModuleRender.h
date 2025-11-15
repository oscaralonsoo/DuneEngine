#pragma once

#include "Module.h"
#include "shader_s.h"
#include <vector>
#include <IL/il.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#define IL_NO_WCHAR_T


class ModuleRender : public Module
{
public:

	ModuleRender();

	// Destructor
	virtual ~ModuleRender();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool Update();
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	//void SetViewPort(const SDL_Rect& rect);
	//void ResetViewPort();

	// Drawing
	// bool DrawTexture(SDL_Texture* texture, int x, int y, const SDL_Rect* section = NULL, float speed = 1.0f, double angle = 0, int pivotX = INT_MAX, int pivotY = INT_MAX) const;
	// bool DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool filled = true, bool useCamera = true) const;
	// bool DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	// bool DrawCircle(int x1, int y1, int redius, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, bool useCamera = true) const;
	// bool DrawText(const char* text, int posX, int posY, int w, int h) const;

	// Set background color
	//void SetBackgroundColor(SDL_Color color);

public:

    Shader* shader = nullptr;
    GLuint VAO = 0, VBO = 0;
    GLuint texture1 = 0, texture2 = 0;
    glm::mat4 model, view, projection;
	// SDL_ModuleRenderer* ModuleRenderer;
	// SDL_Rect camera;
	// SDL_Rect viewport;
	// SDL_Color background;
	// TTF_Font* font;
};