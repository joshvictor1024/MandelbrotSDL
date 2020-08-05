#include "graphics/graphics_sdl.h"
#include "graphics/color.h"

namespace mdb {
    
/***************************************************************
    graphics.h
***************************************************************/

bool Initiallized()
{
    return ((renderer != nullptr) && (drawArea != nullptr));
}

void SetDrawAreaAsTarget() { SDL_SetRenderTarget(renderer, drawArea); }
void UnsetDrawAreaAsTarget() { SDL_SetRenderTarget(renderer, nullptr); }

// According to SDL_PIXELFORMAT_ARGB8888

constexpr int PixelSize() noexcept { return 4; }
constexpr int PixelOffsetR() noexcept { return 2; }
constexpr int PixelOffsetG() noexcept { return 1; }
constexpr int PixelOffsetB() noexcept { return 0; }

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

/***************************************************************
    graphics_sdl.h
***************************************************************/

SDL_Renderer* renderer = nullptr;
SDL_Texture* drawArea = nullptr;

SDLTexture::SDLTexture(std::uint_fast16_t width, std::uint_fast16_t height, SDL_TextureAccess access, SDL_PixelFormatEnum format) :
    width(width), height(height)
{
    texture = SDL_CreateTexture(renderer, format, access, width, height);
    pixelData.resize(width * height * PixelSize() * PixelSize());
}

SDLTexture::~SDLTexture()
{
    SDL_DestroyTexture(texture);
}

//PixelData_t* SDLTexture::Lock()
//{
//    PixelData_t* pixelData;
//    int pitchDiscarded;
//
//    SDL_LockTexture(texture, nullptr, (void**)(&pixelData), &pitchDiscarded);
//
//    return pixelData;
//}
//
//void SDLTexture::Unlock()
//{
//    SDL_UnlockTexture(texture);
//}
//
//void SDLTexture::Update(PixelData_t* pixelData, int rowSize)
//{
//    SDL_UpdateTexture(texture, NULL, pixelData, rowSize * PixelSize());
//}

void SDLTexture::Draw(RectI srcRect, RectF dstRect)
{
    SDL_Rect src = { srcRect.x, srcRect.y, srcRect.w, srcRect.h };
    SDL_FRect dst = { dstRect.x, dstRect.y, dstRect.w, dstRect.h };
    SDL_RenderCopyF(renderer, texture, &src, &dst);
}

void SDLTexture::SetAsTarget()
{
    SDL_SetRenderTarget(renderer, texture);
}

void SDLTexture::UnsetAsTarget()
{
    SDL_SetRenderTarget(renderer, nullptr);
}

void SDLTexture::Color(int u, int v, Iteration_t iteration, Iteration_t threshold)
{
    PixelDataIndex_t index = (u + v * width) * PixelSize();
    if (iteration == threshold)
    {
        pixelData[index + PixelOffsetR()] = BOUNDED_COLOR.r * 0xff;
        pixelData[index + PixelOffsetG()] = BOUNDED_COLOR.g * 0xff;
        pixelData[index + PixelOffsetB()] = BOUNDED_COLOR.b * 0xff;
    }
    else
    {
        mdb::Color color = getInterpolated(iteration % COLOR_COUNT);

        pixelData[index + PixelOffsetR()] = color.r * 0xff;
        pixelData[index + PixelOffsetG()] = color.g * 0xff;
        pixelData[index + PixelOffsetB()] = color.b * 0xff;
    }
}

void SDLTexture::Update()
{
    SDL_UpdateTexture(texture, NULL, pixelData.data(), width * PixelSize());
}

} // namespace mdb