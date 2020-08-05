#include "mandelbrot.h"
#include "graphics/graphics_sdl.h"

#include "sdl_manager.h"

/***************************************************************
    Graphics
***************************************************************/

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

constexpr int MOVEPIXEL_PER_MS = 200;

constexpr int MAX_FRAMERATE = 50;
constexpr int MIN_FRAMETIME = 1000 / MAX_FRAMERATE;

constexpr mdb::Number_t DEFAULT_X = -2.4;
constexpr mdb::Number_t DEFAULT_Y = 1.075;
constexpr mdb::Number_t DEFAULT_PIXEL_LENGTH = 0.003;
constexpr mdb::Iteration_t DEFAULT_THRESHOLD = 256;

constexpr float SCROLL_ZOOM_OUT = 1.0f / 0.8f;  // Deprecated

constexpr float SCROLL_ZOOM_DIFFERENCE = 0.25f;

constexpr float MAX_PIXEL_LENGTH = 0.005f;

//constexpr int CHANNEL = 3;
// TODO: check if it still works
//void convertToPng(SDL_Texture* tex, const char* fileName)
//{
//	std::vector<std::future<void>> futures;
//	futures.reserve(WINDOW_HEIGHT);
//
//	uint8_t* pixelsTex;
//	uint8_t* pixelsPng = new uint8_t[WINDOW_WIDTH * WINDOW_HEIGHT * CHANNEL];
//	int pitchDiscarded;
//
//	SDL_LockTexture(tex, nullptr, (void**)(&pixelsTex), &pitchDiscarded);
//
//	int pixel = 0;
//	for (int y = 0; y < WINDOW_HEIGHT; y++, pixel+= WINDOW_WIDTH)
//	{
//		auto convertToPngSub = [pixelsTex, pixelsPng](int pixel) {
//
//			for (int x = 0; x < WINDOW_WIDTH; x++, pixel++)
//			{
//				pixelsPng[pixel * CHANNEL + 0] = pixelsTex[pixel * TEXTURE_PITCH + PIXELDATA_OFFSETR];
//				pixelsPng[pixel * CHANNEL + 1] = pixelsTex[pixel * TEXTURE_PITCH + PIXELDATA_OFFSETG];
//				pixelsPng[pixel * CHANNEL + 2] = pixelsTex[pixel * TEXTURE_PITCH + PIXELDATA_OFFSETB];
//			}
//		};
//		futures.push_back(std::async(std::launch::async, convertToPngSub, pixel));
//	}
//
//	stbi_write_png(fileName, WINDOW_WIDTH, WINDOW_HEIGHT, CHANNEL, pixelsPng, WINDOW_WIDTH * CHANNEL);
//	delete[] pixelsPng;
//}

/***************************************************************
    Program
***************************************************************/

int main(int argc, char* argv[])
{
    {
        // Setting up SDL

        SDLManager sdl(WINDOW_WIDTH, WINDOW_HEIGHT);
        sdl.PrintRendererInfo();

        SDL_Texture* screen = SDL_CreateTexture(sdl.Renderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);

        // Setting up libmandelbrot

        mdb::SetRenderer(sdl.Renderer());
        mdb::SetDrawAreaTexture(screen);

        if (mdb::Initiallized() == false)
        {
            MDB_ERROR("Graphics is not initiallized!");
        }

        mdb::Number_t originX = DEFAULT_X;
        mdb::Number_t originY = DEFAULT_Y;
        mdb::Number_t pixelLength = DEFAULT_PIXEL_LENGTH;
        mdb::Iteration_t threshold = DEFAULT_THRESHOLD;

        mdb::Scene scene({ 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT }, pixelLength);
        scene.SetNumberRange(originX, originY, pixelLength);

        // Controls

        int mouseX = 0;
        int mouseY = 0;
        int mouseDownX = 0;
        int mouseDownY = 0;
        bool mouseButtonDown = false;
        bool keyUp = false;
        bool keyDown = false;
        bool keyLeft = false;
        bool keyRight = false;
        bool keyZoomIn = false;
        bool keyZoomOut = false;
        bool keyIterationUpPressed = false;
        bool keyIterationDownPressed = false;

        // Program

        SDL_Event e;
        uint32_t lastFrameTick = SDL_GetTicks();
        uint32_t lastFrameTime = 0;
        bool shouldRender = false;
        bool run = true;

        while (run)
        {
            SDL_WaitEventTimeout(&e, MIN_FRAMETIME);

            // Event

            switch (e.type)
            {
            case SDL_QUIT:

                run = false;
                break;

                // TODO: revive mousewheel (with sdl)
                //case SDL_MOUSEWHEEL:

                //	if (mouseButtonDown == false && shouldRender == false)
                //	{
                //		Number_t oldPixelLength = pixelLength;

          //              int x;
          //              int y;
          //              SDL_GetMouseState(&x, &y);

          //              if (e.wheel.y > 0 && mouseWheelDirectionDown)  // Up: zoom out, pixelLength increases
          //              {
          //                  mouseWheelDirectionDown = false;
          //                  scene.Zoom(x, y, SCROLL_ZOOM_OUT);
          //              }
          //              else if (e.wheel.y < 0 && mouseWheelDirectionDown == false)
          //              {
          //                  mouseWheelDirectionDown = true;
          //                  scene.Zoom(x, y, 1.0 / SCROLL_ZOOM_OUT);
          //              }

                //		shouldRender = true;
                //	}

                //	break;

            case SDL_MOUSEBUTTONDOWN:

                mouseX = e.button.x;
                mouseY = e.button.y;
                if (e.button.button & SDL_BUTTON_LEFT && mouseButtonDown == false)
                {
                    mouseButtonDown = true;

                    mouseX = e.button.x;
                    mouseY = e.button.y;
                    mouseDownX = e.button.x;
                    mouseDownY = e.button.y;
                    //RLOG("down");
                }

                break;

            case SDL_MOUSEBUTTONUP:

                mouseX = e.button.x;
                mouseY = e.button.y;
                if (e.button.button & SDL_BUTTON_LEFT && mouseButtonDown)
                {
                    mouseButtonDown = false;

                    scene.Movement(mouseDownX - mouseX, mouseDownY - mouseY);
                    //RLOG("up");

                    shouldRender = true;
                }

                break;

            case SDL_MOUSEMOTION:
                mouseX = e.button.x;
                mouseY = e.button.y;
                break;

            case SDL_KEYDOWN:

                //if (e.key.keysym.scancode == SDL_SCANCODE_F2 && e.key.repeat == 0)
                //{
                //	std::stringstream ss;
                //	ss << std::setprecision(10) <<
                //		"pos(" << originX << "," << originY << ")-pxL(" << pixelLength << ").png";
                //	convertToPng(tex, ss.str().c_str());
                //}

                switch (e.key.keysym.scancode)
                {
                case SDL_SCANCODE_ESCAPE: run = false; break;
                case SDL_SCANCODE_UP: keyUp = true; break;
                case SDL_SCANCODE_DOWN: keyDown = true; break;
                case SDL_SCANCODE_LEFT: keyLeft = true; break;
                case SDL_SCANCODE_RIGHT: keyRight = true; break;
                case SDL_SCANCODE_Z: keyZoomOut = true; break;
                case SDL_SCANCODE_X: keyZoomIn = true; break;

                case SDL_SCANCODE_A:
                    if (keyIterationUpPressed == false)
                    {
                        keyIterationUpPressed = true;
                        threshold += 128;
                        MDB_INFO("up to {}", threshold);
                        scene.Recompute();
                    }
                    break;

                case SDL_SCANCODE_S:
                    if (keyIterationDownPressed == false)
                    {
                        keyIterationDownPressed = true;
                        if (threshold > 128)
                        {
                            threshold -= 128;
                            MDB_INFO("down to {}", threshold);
                            scene.Recompute();
                        }
                    }
                    break;
                }

                shouldRender = true;

                break;

            case SDL_KEYUP:

                switch (e.key.keysym.scancode)
                {
                case SDL_SCANCODE_UP: keyUp = false; break;
                case SDL_SCANCODE_DOWN: keyDown = false; break;
                case SDL_SCANCODE_LEFT: keyLeft = false; break;
                case SDL_SCANCODE_RIGHT: keyRight = false; break;
                case SDL_SCANCODE_Z: keyZoomOut = false; break;
                case SDL_SCANCODE_X: keyZoomIn = false; break;
                
                case SDL_SCANCODE_A:
                    if (keyIterationUpPressed)
                    {
                        keyIterationUpPressed = false;
                    }
                    break;

                case SDL_SCANCODE_S:
                    if (keyIterationDownPressed)
                    {
                        keyIterationDownPressed = false;
                    }
                    break;
                }

                shouldRender = true;

                break;
            }

            if (SDL_GetTicks() - lastFrameTick > MIN_FRAMETIME)
            {
                lastFrameTime = SDL_GetTicks() - lastFrameTick;
                lastFrameTick = SDL_GetTicks();

                float movement = MOVEPIXEL_PER_MS * lastFrameTime * 0.001;
                if (keyUp) { scene.Movement(0, -1 * movement); }
                if (keyDown) { scene.Movement(0, movement); }
                if (keyLeft) { scene.Movement(-1 * movement, 0); }
                if (keyRight) { scene.Movement(movement, 0); }

                if (keyZoomOut) { scene.Zoom(mouseX, mouseY, 1.01); }
                if (keyZoomIn) { scene.Zoom(mouseX, mouseY, 1.0 / 1.01); }

                // Render

                SDL_RenderClear(sdl.Renderer());

                scene.Update(threshold);
                scene.RenderCopy();
                SDL_RenderCopy(sdl.Renderer(), screen, nullptr, nullptr);
                scene.DebugCopy({ 0, 0, 20 * 16, 13 * 16 });
                //scene.DebugCopy();
                SDL_RenderPresent(sdl.Renderer());

                shouldRender = false;
            }
        }

        SDL_DestroyTexture(screen);
    }

	return 0;
}