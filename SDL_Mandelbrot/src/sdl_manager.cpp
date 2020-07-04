#include "sdl_manager.h"
#include "constants.h"

SDLManager SDLManager::instance;

SDLManager::SDLManager()
{
    bool success = true;
    DLOG("SDL_Init!");

    // Initialize SDL

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        DLOG("SDL could not initialize! SDL Error: " << SDL_GetError());
        success = false;
    }

    // Set texture filtering to linear

    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == SDL_FALSE)
    {
        RLOG("Warning: Linear texture filtering not enabled!");
    }

    // Create window

    mWindow = SDL_CreateWindow(
        "Mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (mWindow == nullptr)
    {
        DLOG("Window could not be created! SDL Error: " << SDL_GetError());
        success = false;
    }

    // Create renderer for window
    // Set VSync here

    mRenderer = SDL_CreateRenderer(
        mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    //window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (mRenderer == nullptr)
    {
        DLOG("Renderer could not be created! SDL Error: " << SDL_GetError());
        success = false;
    }
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);

    // Stop emitting SDL_TEXTEDITING etc. events

    SDL_StopTextInput();

    if (success == false) exit(-1);
}

SDLManager::~SDLManager()
{
    SDL_Quit();
    DLOG("SDL_Quit!");
}