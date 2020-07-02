#include <complex>              // For computation
#include <vector>               // For async
#include <future>
#include <sstream>              // For screenshot filename
#include <iomanip>              // For position logging

#include "stb_image_write.h"    // For screenshot

#include "engine.h"
#include "constants.h"
#include "logging.h"

// Types ///////////////////////////////////////////////////////

typedef double Position_t;
typedef std::complex<Position_t> Complex_t;
typedef uint16_t Iteration_t;

// Utility ///////////////////////////////////////////////////////

constexpr Position_t absSquared(const Complex_t& c)
{
	return (c.real() * c.real() + c.imag() + c.imag());
}

// Computation ///////////////////////////////////////////////////////

void compute(
    Iteration_t* iterations,
    std::vector<std::future<void>>& futures,
	const Position_t originX,
	const Position_t originY,
	const Position_t pixelLength,
	const Iteration_t threshold)
{
	int pos = 0;

    // Pixel data starts from top left
    // Dividing window into 1-pixel-high lines for async computation

	for (int y = WINDOW_HEIGHT / 2; y > -1 * WINDOW_HEIGHT / 2; --y, pos += WINDOW_WIDTH)
	{
		auto mandelbrotSub = [iterations, originX, originY, pixelLength, threshold, y](int pos) {

			for (int x = -1 * WINDOW_WIDTH / 2; x < WINDOW_WIDTH / 2; ++x, ++pos)
			{
				Complex_t c = { 0.0, 0.0 };
				Complex_t dc =
                {
					x * pixelLength + originX,
					y * pixelLength + originY
				};

                Iteration_t it = 0;
				for (; it < threshold; ++it)
				{
					c = c * c + dc;

					if (absSquared(c) > static_cast<Position_t>(2 * 2))
					{
						break;
					}
				}
                iterations[pos] = it;
			}
		};

		futures.push_back(std::async(std::launch::async, mandelbrotSub, pos));
	}
}

// Graphics ///////////////////////////////////////////////////////

void updateTexture(
    SDL_Texture* tex,
    Iteration_t* iterations)
{
    uint8_t* pixelsTex;
    int pitchDiscarded;

    SDL_LockTexture(tex, nullptr, (void**)(&pixelsTex), &pitchDiscarded);    
    for (int pos = 0; pos < WINDOW_WIDTH * WINDOW_HEIGHT; ++pos)
    {
        color(pixelsTex, pos * RGB888_SIZE, iterations[pos]);
    }
    SDL_UnlockTexture(tex);
}

void mandelbrot(
    SDL_Texture* tex,
    Iteration_t* iterations,
	const Position_t originX,
	const Position_t originY,
	const Position_t pixelLength,
	const int threshold)
{
    std::vector<std::future<void>> futures;
    futures.reserve(WINDOW_HEIGHT);
    compute(iterations, futures, originX, originY, pixelLength, threshold);
    for (auto& i : futures)
    {
        i.get();
    }

    updateTexture(tex, iterations);
}

// TODO: check if it still works
void convertToPng(SDL_Texture* tex, const char* fileName)
{
	std::vector<std::future<void>> futures;
	futures.reserve(WINDOW_HEIGHT);

	uint8_t* pixelsTex;
	uint8_t* pixelsPng = new uint8_t[WINDOW_WIDTH * WINDOW_HEIGHT * CHANNEL];
	int pitchDiscarded;

	SDL_LockTexture(tex, nullptr, (void**)(&pixelsTex), &pitchDiscarded);

	int pixel = 0;
	for (int y = 0; y < WINDOW_HEIGHT; y++, pixel+= WINDOW_WIDTH)
	{
		auto convertToPngSub = [pixelsTex, pixelsPng](int pixel) {

			for (int x = 0; x < WINDOW_WIDTH; x++, pixel++)
			{
				pixelsPng[pixel * CHANNEL + 0] = pixelsTex[pixel * RGB888_SIZE + RGB888_OFFSETR];
				pixelsPng[pixel * CHANNEL + 1] = pixelsTex[pixel * RGB888_SIZE + RGB888_OFFSETG];
				pixelsPng[pixel * CHANNEL + 2] = pixelsTex[pixel * RGB888_SIZE + RGB888_OFFSETB];
			}
		};
		futures.push_back(std::async(std::launch::async, convertToPngSub, pixel));
	}

	stbi_write_png(fileName, WINDOW_WIDTH, WINDOW_HEIGHT, CHANNEL, pixelsPng, WINDOW_WIDTH * CHANNEL);
	delete[] pixelsPng;
}

// Program ///////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	init(WINDOW_WIDTH, WINDOW_HEIGHT);

    Iteration_t* iterations = new Iteration_t[WINDOW_WIDTH * WINDOW_HEIGHT];
	SDL_Texture* tex = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

	Position_t originX = 0.0;
	Position_t originY = 0.0;
	Position_t pixelLength = 0.003;

    mandelbrot(tex, iterations, originX, originY, pixelLength, THRESHOLD);
	SDL_RenderCopy(gSDLRenderer, tex, nullptr, nullptr);
	SDL_RenderPresent(gSDLRenderer);

	SDL_Event e;

	uint32_t lastRenderTime = SDL_GetTicks();

	int mouseStartX = 0;
	int mouseStartY = 0;
	bool mouseButtonDown = false;

	bool shouldRender = false;
	bool run = true;

	while (SDL_WaitEvent(&e) && run)
	{
        // Event

		switch (e.type)
		{
		case SDL_QUIT:
			run = false;
            break;

		case SDL_MOUSEWHEEL:

			if (!mouseButtonDown)
			{
				Position_t oldPixelLength = pixelLength;

				if (e.wheel.y > 0)//up
					pixelLength *=  Position_t(0.8);
				else
					pixelLength *= (1.0 / 0.8);

				if (pixelLength < MAX_PIXEL_LENGTH)
				{
					int mouseX;
					int mouseY;
					SDL_GetMouseState(&mouseX, &mouseY);

					originX += static_cast<Position_t>(WINDOW_WIDTH / 2 - mouseX)* (pixelLength - oldPixelLength);
					originY += static_cast<Position_t>(mouseY - WINDOW_HEIGHT / 2)* (pixelLength - oldPixelLength);

					shouldRender = true;
				}
				else
				{
					pixelLength = oldPixelLength;
				}
			}

			break;

		case SDL_MOUSEBUTTONDOWN:

			if (e.button.button & SDL_BUTTON_LEFT)
			{
				mouseButtonDown = true;

				mouseStartX = e.button.x;
				mouseStartY = e.button.y;
				//RLOG(mouseStartX << mouseStartY << " down");
			}

			break;

		case SDL_MOUSEBUTTONUP:

			if (e.button.button & SDL_BUTTON_LEFT)
			{
				mouseButtonDown = false;

				originX += static_cast<Position_t>(mouseStartX - e.button.x) * pixelLength;
				originY += static_cast<Position_t>(e.button.y - mouseStartY) * pixelLength;

				//RLOG(e.button.x << e.button.y << " up");

				shouldRender = true;
			}

			break;

		//case SDL_MOUSEMOTION:
		//	if (mouseButtonDown)
		//	{
		//		//mouseMoved = true;
		//	}
		//	break;

		case SDL_KEYDOWN:

			if (e.key.keysym.scancode == SDL_SCANCODE_F2 && !(e.key.repeat))
			{
				std::stringstream ss;
				ss << std::setprecision(10) <<
					"pos(" << originX << "," << originY << ")-pxL(" << pixelLength << ").png";
				convertToPng(tex, ss.str().c_str());
			}

            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
            {
                run = false;
            }

			break;
		}

        // Render

		if ((SDL_GetTicks() - lastRenderTime > (1000 / MAX_FRAMERATE)) && shouldRender)
		{
            mandelbrot(tex, iterations, originX, originY, pixelLength, THRESHOLD);
			SDL_RenderCopy(gSDLRenderer, tex, nullptr, nullptr);
			SDL_RenderPresent(gSDLRenderer);

			RLOG(std::setprecision(10) << "origin: (" << originX << ", " << originY << ") screen width: " << pixelLength * WINDOW_WIDTH);

			shouldRender = false;
		}
	}

    delete[] iterations;

	SDL_DestroyTexture(tex);
	quit();

	return 0;
}