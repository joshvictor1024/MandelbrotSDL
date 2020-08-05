#include "graphics/graphics_olc.h"
#include "graphics/color.h"

namespace mdb {

    /***************************************************************
        graphics.h
    ***************************************************************/

    bool Initiallized()
    {
        return ((engine != nullptr) && (drawArea != nullptr));
    }

    void SetDrawAreaAsTarget() { engine->SetDrawTarget(drawArea); }
    void UnsetDrawAreaAsTarget() { engine->SetDrawTarget(nullptr); }

    std::unique_ptr<Texture> Texture::Create(int width, int height, Access access, Format format)
    {
        return std::make_unique<OLCDecal>(width, height);
    }

    /***************************************************************
        graphics_sdl.h
    ***************************************************************/

    olc::PixelGameEngine* engine = nullptr;
    olc::Sprite* drawArea = nullptr;

    OLCDecal::OLCDecal(std::uint_fast16_t width, std::uint_fast16_t height) :
        sprite({ static_cast<int32_t>(width), static_cast<int32_t>(height) }),
        decal({ &sprite }) {}

    void OLCDecal::Draw(RectI srcRect, RectF dstRect)
    {
        engine->DrawPartialDecal(
            { dstRect.x, dstRect.y }, { dstRect.w, dstRect.h }, &decal,
            { static_cast<float>(srcRect.x), static_cast<float>(srcRect.y) }, { static_cast<float>(srcRect.w), static_cast<float>(srcRect.h) }
        );
    }

    void OLCDecal::SetAsTarget()
    {
        engine->SetDrawTarget(&sprite);
    }

    void OLCDecal::UnsetAsTarget()
    {
        engine->SetDrawTarget(nullptr);
        Update();
    }

    void OLCDecal::Color(int u, int v, Iteration_t iteration, Iteration_t threshold)
    {
        olc::Pixel* pixel = sprite.GetData();
        pixel = &(pixel[u + v * (sprite.width)]);

        if (iteration == threshold)
        {
            pixel->r = BOUNDED_COLOR.r * 0xff;
            pixel->g = BOUNDED_COLOR.g * 0xff;
            pixel->b = BOUNDED_COLOR.b * 0xff;
        }
        else
        {
            mdb::Color color = getInterpolated(iteration % COLOR_COUNT);

            pixel->r = color.r * 0xff;
            pixel->g = color.g * 0xff;
            pixel->b = color.b * 0xff;
        }
    }

    void OLCDecal::Update()
    {
        decal.Update();
    }

} // namespace mdb