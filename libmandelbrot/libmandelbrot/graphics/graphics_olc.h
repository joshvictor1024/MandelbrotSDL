#ifndef GRAPHICS_OLC_H
#define GRAPHICS_OLC_H

#include "olcPixelGameEngine.h" // requires olcPGE 2.0+ for olc::Decal
#include "graphics.h"

namespace mdb {

extern olc::PixelGameEngine* engine;
extern olc::Sprite* drawArea;

// Non-owning
inline void SetEngine(olc::PixelGameEngine* engine)
{
    mdb::engine = engine;
}

// Non-owning
inline void SetDrawAreaSprite(olc::Sprite* screen)
{
    mdb::drawArea = screen;
}

class OLCDecal final : public Texture
{
public:

    OLCDecal(std::uint_fast16_t width, std::uint_fast16_t height);

    void Draw(RectI srcRect, RectF dstRect) override;
    void SetAsTarget() override;    // No decal to decal rendering in olc; falling back to software rendering
    void UnsetAsTarget() override;  // No decal to decal rendering in olc; falling back to software rendering

    void Color(int u, int v, Iteration_t iteration, Iteration_t threshold) override;
    void Update() override;

private:

    olc::Sprite sprite;
    olc::Decal decal;
};

} // namespace mdb

#endif // !GRAPHICS_OLC_H
