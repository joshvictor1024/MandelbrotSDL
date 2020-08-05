#ifndef SDL_MANAGER_H
#define SDL_MANAGER_H

#include <SDL.h>
#include "log.h"

// Simple usage of SDL
// Should only be instantiated once
// Such check is not put in place
class SDLManager
{
public:

    SDLManager(int windowWidth, int windowHeight);
    ~SDLManager();

    [[nodiscard]] SDL_Window* Window() const noexcept { return window; }
    [[nodiscard]] SDL_Renderer* Renderer() const noexcept { return renderer; }

    // Only prints below log level info
    inline void PrintRendererInfo() const
    {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(renderer, &info);

        MDB_INFO("");
        MDB_INFO("Renderer Info:");

        MDB_INFO("Name: {}", info.name);

        MDB_INFO("Flags:");
        if (info.flags & SDL_RENDERER_SOFTWARE) { MDB_INFO("  SDL_RENDERER_SOFTWARE"); }
        if (info.flags & SDL_RENDERER_ACCELERATED) { MDB_INFO("  SDL_RENDERER_ACCELERATED"); }
        if (info.flags & SDL_RENDERER_PRESENTVSYNC) { MDB_INFO("  SDL_RENDERER_PRESENTVSYNC"); }
        if (info.flags & SDL_RENDERER_TARGETTEXTURE) { MDB_INFO("  SDL_RENDERER_TARGETTEXTURE"); }

        MDB_INFO("Texture Formats:");
        for (int i = 0; i < info.num_texture_formats; ++i)
        {
            switch (info.texture_formats[i])
            {
            case SDL_PIXELFORMAT_ARGB8888:  MDB_INFO("  SDL_PIXELFORMAT_ARGB8888"); break;
            case SDL_PIXELFORMAT_YV12:      MDB_INFO("  SDL_PIXELFORMAT_YV12"); break;
            case SDL_PIXELFORMAT_IYUV:      MDB_INFO("  SDL_PIXELFORMAT_IYUV"); break;
            default:                        MDB_INFO("  unknown"); break;
            }
        }

        MDB_INFO("Max Texture Size: {} * {}", info.max_texture_width, info.max_texture_height);
        MDB_INFO("");
    }

private:

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};

#endif //!SDL_MANAGER_H