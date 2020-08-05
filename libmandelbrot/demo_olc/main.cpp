#include "mandelbrot.h"
#include "graphics/graphics_olc.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <string>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

class Example : public olc::PixelGameEngine
{
public:

    Example()
    {
        sAppName = "Mandelbrot";
    }

private:

    std::unique_ptr<olc::Sprite> fractal;

    std::unique_ptr<mdb::Scene> scene;

    // Fractal Parameter Defaults

    mdb::Number_t originX = -2.4;
    mdb::Number_t originY = 1.075;
    mdb::Number_t pixelLength = 0.003;
    mdb::Iteration_t threshold = 256;

    // Controls

    olc::vi2d mouseDown;
    constexpr static int MOVEPIXEL_PER_SEC = 200;
    constexpr static float ZOOM_PER_FRAME = 1.005;

public:

    bool OnUserCreate() override
    {
        // Setting up olc draw surface

        fractal = std::make_unique<olc::Sprite>(WINDOW_WIDTH, WINDOW_HEIGHT);

        // Setting up libmandelbrot

        mdb::SetEngine(this);
        mdb::SetDrawAreaSprite(fractal.get());

        if (mdb::Initiallized() == false)
        {
            MDB_ERROR("Graphics is not initiallized!");
        }

        scene = std::make_unique<mdb::Scene>(mdb::RectI{ 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT }, pixelLength);
        scene->SetNumberRange(originX, originY, pixelLength);

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        // KeyBoard

        if (GetKey(olc::Key::ESCAPE).bPressed) return false;

        float movement = MOVEPIXEL_PER_SEC * fElapsedTime;
        if (GetKey(olc::Key::UP).bHeld)     scene->Movement(0, -1 * movement);
        if (GetKey(olc::Key::DOWN).bHeld)   scene->Movement(0, movement);
        if (GetKey(olc::Key::LEFT).bHeld)   scene->Movement(-1 * movement, 0);
        if (GetKey(olc::Key::RIGHT).bHeld)  scene->Movement(movement, 0);

        if (GetKey(olc::Key::Z).bHeld) { scene->Zoom(GetMousePos().x, GetMousePos().y, ZOOM_PER_FRAME); }
        if (GetKey(olc::Key::X).bHeld) { scene->Zoom(GetMousePos().x, GetMousePos().y, 1.0 / ZOOM_PER_FRAME); }

        if (GetKey(olc::Key::A).bPressed)
        {
            threshold += 128;
            scene->Recompute();
        }
        if (GetKey(olc::Key::S).bPressed)
        {
            if (threshold > 128)
            {
                threshold -= 128;
                scene->Recompute();
            }
        }

        // Mouse

        auto LeftMouse = GetMouse(0);

        if (LeftMouse.bPressed)
        {
            mouseDown = GetMousePos();
        }
        else if (LeftMouse.bReleased)
        {
            scene->Movement(mouseDown.x - GetMousePos().x, mouseDown.y - GetMousePos().y);
        }

        // Draw

        scene->Update(threshold);
        scene->Draw();
        scene->DebugDraw({ 0, 0, 20 * 16, 13 * 16 });

        auto digit = [](int n) {
            return char('0' + n);
        };

        // up to 99999
        auto numberString = [digit](unsigned int n) {
            std::string s;
            s += digit(n / 10000);
            s += digit(n / 1000 % 10);
            s += digit((n / 100) % 10);
            s += digit((n / 10) % 10);
            s += digit(n % 10);
            return s;
        };

        // As per PGE2.07, decals will always be drawn AFTER sprite of layer
        // See olcPixelGameEngine.h line 2618 and on
        // Easiest workaround is to use the Decal draw functions instead of the original Sprite conterparts
        // e.g. DrawStringDecal() instead of DrawString
        // However, this costs a lot of memory
        DrawStringDecal({ 8, WINDOW_HEIGHT - 32 }, "Threshold: " + numberString(threshold), olc::WHITE, { 2, 2 });
        
        return true;
    }
};

int main()
{
    Example example;
    if (example.Construct(WINDOW_WIDTH, WINDOW_HEIGHT, 1, 1))
        example.Start();

    return 0;
}