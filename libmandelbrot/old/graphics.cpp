#include "graphics.h"

// Include all implementation headers here
#if GRAPHICS == GRAPHICS_NONE
#error Not supported
#elif GRAPHICS == GRAPHICS_SDL

#include "graphics_sdl.h"

namespace mdb {

std::unique_ptr<Texture> Texture::Create(int width, int height, Access access, Format format)
{
    return std::make_unique<SDLTexture>(
        width,
        height,
        [access]()
        {
            switch (access)
            {
            case Texture::Access::STATIC: return SDL_TEXTUREACCESS_STATIC;
            case Texture::Access::STREAMING: return SDL_TEXTUREACCESS_STREAMING;
            case Texture::Access::TARGET: return SDL_TEXTUREACCESS_TARGET;
                //default: break; //TODO: assert
            }
        }(),
        [format]()
        {
            switch (format)
            {
            case Texture::Format::NATIVE: return SDL_PIXELFORMAT_ARGB8888;
                //default: break; //TODO: assert
            }
        }()
    );
}

} // namespace mdb

#endif // GRAPHICS

//std::unique_ptr<Graphics>* Texture::graphics = nullptr;

//std::unique_ptr<Graphics> Graphics::Create()
//{
//    #if GRAPHICS == GRAPHICS_NONE
//    #error Not supported
//    #elif GRAPHICS == GRAPHICS_SDL
//
//    return std::make_unique<SDLGraphics>();
//
//    #endif // GRAPHICS
//}


//std::unique_ptr<Texture> Texture::Create(int width, int height, Access access, Format format)
//{
//    #if GRAPHICS == GRAPHICS_NONE
//    #error Not supported
//    #elif GRAPHICS == GRAPHICS_SDL
//
//    return std::make_unique<SDLTexture>(
//        width,
//        height,
//        [access]()
//        {
//            switch (access)
//            {
//            case Texture::Access::STATIC: return SDL_TEXTUREACCESS_STATIC;
//            case Texture::Access::STREAMING: return SDL_TEXTUREACCESS_STREAMING;
//            case Texture::Access::TARGET: return SDL_TEXTUREACCESS_TARGET;
//                //default: break; //TODO: assert
//            }
//        }(),
//        [format]()
//        {
//            switch (format)
//            {
//            case Texture::Format::NATIVE: return SDL_PIXELFORMAT_ARGB8888;
//                //default: break; //TODO: assert
//            }
//        }()
//    );
//
//    #endif // GRAPHICS
//}