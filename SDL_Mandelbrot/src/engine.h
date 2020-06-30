#ifndef ENGINE_H
#define ENGINE_H

#include <SDL.h>
#include "logging.h"

//FIX: may be re-defined
SDL_Window* gSDLWindow = nullptr;
SDL_Renderer* gSDLRenderer = nullptr;

void init(int windowedWidth, int windowedHeight)
{
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		DLOG("SDL could not initialize! SDL Error: " << SDL_GetError());
		success = false;
	}
	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		RLOG("Warning: Linear texture filtering not enabled!");
	}

	//Create window
	gSDLWindow = SDL_CreateWindow(
		"Mandelbrot alpha v0.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		windowedWidth, windowedHeight, SDL_WINDOW_SHOWN);
	if (!gSDLWindow)
	{
		DLOG("Window could not be created! SDL Error: " << SDL_GetError());
		success = false;
	}

	//Create renderer for window
	gSDLRenderer = SDL_CreateRenderer(
		gSDLWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		//mSDLWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (!gSDLRenderer)
	{
		DLOG("Renderer could not be created! SDL Error: " << SDL_GetError());
		success = false;
	}
	
	SDL_SetRenderDrawColor(gSDLRenderer, 0, 0, 0, 255);

	SDL_StopTextInput();//to stop emitting SDL_TEXTEDITING etc. events

	if (!success) exit(-1);
}
void quit()
{
	SDL_Quit();
}

#endif //!ENGINE_H