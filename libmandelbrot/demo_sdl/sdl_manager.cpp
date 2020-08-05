#include "sdl_manager.h"
#include "log.h"

SDLManager::SDLManager(int windowWidth, int windowHeight)
{
    bool success = true;
    MDB_INFO("SDL_Init!");

    // Initialize SDL

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        MDB_ERROR("SDL could not initialize! SDL Error: {}", SDL_GetError());
        success = false;
    }

    // Set texture filtering to linear

    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == SDL_FALSE)
    {
        MDB_WARN("Warning: Linear texture filtering not enabled!");
    }

    // Create window

    window = SDL_CreateWindow(
        "Mandelbrot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        MDB_ERROR("Window could not be created! SDL Error: {}", SDL_GetError());
        success = false;
    }

    // Create renderer for window
    // Set VSync here

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE
    );
    //window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (renderer == nullptr)
    {
        MDB_ERROR("Renderer could not be created! SDL Error: {}", SDL_GetError());
        success = false;
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Stop emitting SDL_TEXTEDITING etc. events

    SDL_StopTextInput();

    if (success == false) exit(-1);
}

SDLManager::~SDLManager()
{
    SDL_Quit();
    MDB_INFO("SDL_Quit!");
}