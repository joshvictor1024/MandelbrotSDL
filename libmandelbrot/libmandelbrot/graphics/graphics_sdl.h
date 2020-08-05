#ifndef GRAPHICS_SDL_H
#define GRAPHICS_SDL_H

#include <SDL.h>
#include "graphics.h"

namespace mdb {

extern SDL_Renderer* renderer;
extern SDL_Texture* drawArea;

// Non-owning
inline void SetRenderer(SDL_Renderer* renderer)
{
    mdb::renderer = renderer;
}

// Non-owning
inline void SetDrawAreaTexture(SDL_Texture* drawArea)
{
    mdb::drawArea = drawArea;
}

class SDLTexture final : public Texture
{
public:

    SDLTexture(std::uint_fast16_t width, std::uint_fast16_t height, SDL_TextureAccess access, SDL_PixelFormatEnum format);
    virtual ~SDLTexture();

    void Draw(RectI srcRect, RectF dstRect) override;
    void SetAsTarget() override;
    void UnsetAsTarget() override;

    void Color(int u, int v, Iteration_t iteration, Iteration_t threshold) override;
    void Update() override;

private:

    SDL_Texture* texture = nullptr;
    std::vector<std::uint8_t> pixelData;

    std::uint_fast16_t width = 0;
    std::uint_fast16_t height = 0;
};

} // namespace mdb

#endif // !GRAPHICS_SDL_H
