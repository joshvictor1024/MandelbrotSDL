#include <vector>               // For async
#include <future>
#include <sstream>              // For screenshot filename
#include <iomanip>              // For position logging

#include "stb_image_write.h"    // For screenshot

#include "constants.h"
#include "sdl_manager.h"
#include "scene.h"

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

// Program ///////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    SDL_Texture* screen = SDL_CreateTexture(SDLManager::renderer(), SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, WINDOW_WIDTH, WINDOW_HEIGHT);

    //Number_t originX = -1.0;
    //Number_t originY = 0.5;
    //Number_t pixelLength = 0.0001;

	Number_t originX = DEFAULT_X;
    Number_t originY = DEFAULT_Y;
    Number_t pixelLength = DEFAULT_PIXEL_LENGTH;
    Iteration_t threshold = DEFAULT_THRESHOLD;

    Scene scene({ 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT }, pixelLength);
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
            }

            shouldRender = true;

            break;
		}

		if (SDL_GetTicks() - lastFrameTick > MIN_FRAMETIME)
		{
            lastFrameTime = SDL_GetTicks() - lastFrameTick;
            lastFrameTick = SDL_GetTicks();

            if (keyUp)      { scene.Movement(0, -1 * MOVEPIXEL_PER_MS,  lastFrameTime); }
            if (keyDown)    { scene.Movement(0, MOVEPIXEL_PER_MS,       lastFrameTime); }
            if (keyLeft)    { scene.Movement(-1 * MOVEPIXEL_PER_MS, 0,  lastFrameTime); }
            if (keyRight)   { scene.Movement(MOVEPIXEL_PER_MS, 0,       lastFrameTime); }

            if (keyZoomOut) { scene.Zoom(mouseX, mouseY, 1.01       ); }
            if (keyZoomIn)  { scene.Zoom(mouseX, mouseY, 1.0 / 1.01 ); }

            // Render

            scene.Update(threshold);
            scene.RenderCopy(screen);
            SDL_RenderCopy(SDLManager::renderer(), screen, nullptr, nullptr);
            //scene.DebugCopy();
            SDL_RenderPresent(SDLManager::renderer());

			shouldRender = false;
		}
	}

	SDL_DestroyTexture(screen);

	return 0;
}