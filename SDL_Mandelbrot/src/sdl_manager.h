#ifndef SDL_MANAGER_H
#define SDL_MANAGER_H

#include <SDL.h>
#include "logging.h"

class SDLManager
{
public:

    static SDL_Window* window()
    {
        return instance.mWindow;
    }

    static SDL_Renderer* renderer()
    {
        return instance.mRenderer;
    }

private:

    static SDLManager instance;
    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;
    
    SDLManager();
    ~SDLManager();
};

#endif //!SDL_MANAGER_H