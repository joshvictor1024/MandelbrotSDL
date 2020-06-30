#include <vector>
#include <future>
#include <sstream>
#include <iomanip>

//#include "boost\multiprecision\cpp_bin_float.hpp"
//#include "boost\multiprecision\cpp_complex.hpp"
#include "stb_image_write.h"

#include <complex>
#include "engine.h"
#include "color.h"
#include "logging.h"

//constexpr int PRECISION = 25;
//typedef boost::multiprecision::number<boost::multiprecision::backends::cpp_bin_float<PRECISION>> Position_t;
//typedef boost::multiprecision::cpp_complex<PRECISION> Complex_t;
typedef double Position_t;
typedef std::complex<Position_t> Complex_t;

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr int HALF_WIDTH = WIDTH / 2;
constexpr int HALF_HEIGHT = HEIGHT / 2;

constexpr int MAX_FRAMERATE = 5;
constexpr float MAX_PIXEL_LENGTH = 0.01f;



inline Position_t absSquared(const Complex_t& c)
{
	return (c.real() * c.real() + c.imag() + c.imag());
}

void mandelbrot(
	uint8_t* pixelsTex,
	const Position_t originX,
	const Position_t originY,
	const Position_t pixelLength,
	const int threshold)
{
	std::vector<std::future<void>> futures;
	futures.reserve(HEIGHT);

	int pos = 0;
	for (int y = HALF_HEIGHT; y > -1 * HALF_HEIGHT; y--, pos += WIDTH * RGB888_SIZE)
	{
		auto mandelbrotSub = [pixelsTex, originX, originY, pixelLength, threshold, y](int pos) {

			for (int x = -1 * HALF_WIDTH; x < HALF_WIDTH; x++, pos += RGB888_SIZE)
			{
				pixelsTex[pos + RGB888_OFFSETR] = COLOR_BOUNDED[0];
				pixelsTex[pos + RGB888_OFFSETG] = COLOR_BOUNDED[1];
				pixelsTex[pos + RGB888_OFFSETB] = COLOR_BOUNDED[2];

				Complex_t c = { 0.0, 0.0 };
				Complex_t dc = {
					x * pixelLength + originX,
					y * pixelLength + originY
				};

				for (int it = 0; it < threshold; it++)
				{
					c = c * c + dc;

					if (absSquared(c) > static_cast<Position_t>(2 * 2))
					{
						color(pixelsTex, pos, it);
						break;
					}
				}
			}
		};
		futures.push_back(std::async(std::launch::async, mandelbrotSub, pos));
	}
}
void updateTexture(SDL_Texture* tex,
	const Position_t originX,
	const Position_t originY,
	const Position_t pixelLength,
	const int threshold)
{
	uint8_t* pixelsTex;
	int pitchDiscarded;

	SDL_LockTexture(tex, nullptr, (void**)(&pixelsTex), &pitchDiscarded);
	mandelbrot(pixelsTex, originX, originY, pixelLength, threshold);
	SDL_UnlockTexture(tex);
}
void convertToPng(SDL_Texture* tex, const char* fileName)
{
	std::vector<std::future<void>> futures;
	futures.reserve(HEIGHT);

	uint8_t* pixelsTex;
	uint8_t* pixelsPng = new uint8_t[WIDTH * HEIGHT * CHANNEL];
	int pitchDiscarded;

	SDL_LockTexture(tex, nullptr, (void**)(&pixelsTex), &pitchDiscarded);

	int pixel = 0;
	for (int y = 0; y < HEIGHT; y++, pixel+= WIDTH)
	{
		auto convertToPngSub = [pixelsTex, pixelsPng](int pixel) {

			for (int x = 0; x < WIDTH; x++, pixel++)
			{
				pixelsPng[pixel * CHANNEL + 0] = pixelsTex[pixel * RGB888_SIZE + RGB888_OFFSETR];
				pixelsPng[pixel * CHANNEL + 1] = pixelsTex[pixel * RGB888_SIZE + RGB888_OFFSETG];
				pixelsPng[pixel * CHANNEL + 2] = pixelsTex[pixel * RGB888_SIZE + RGB888_OFFSETB];
			}
		};
		futures.push_back(std::async(std::launch::async, convertToPngSub, pixel));
	}

	stbi_write_png(fileName, WIDTH, HEIGHT, CHANNEL, pixelsPng, WIDTH * CHANNEL);
	delete[] pixelsPng;
}

int main(int argc, char* argv[])
{
	init(WIDTH, HEIGHT);


	SDL_Texture* tex = SDL_CreateTexture(gSDLRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	Position_t originX = 0.0;
	Position_t originY = 0.0;
	Position_t pixelLength = 0.003;
	updateTexture(tex, originX, originY, pixelLength, THRESHOLD);
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
		switch (e.type)
		{
		case SDL_QUIT:
			run = false; break;

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

					originX += static_cast<Position_t>(HALF_WIDTH - mouseX)* (pixelLength - oldPixelLength);
					originY += static_cast<Position_t>(mouseY - HALF_HEIGHT)* (pixelLength - oldPixelLength);

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
			break;
		}		

		if ((SDL_GetTicks() - lastRenderTime > (1000 / MAX_FRAMERATE)) && shouldRender)
		{
			updateTexture(tex, originX, originY, pixelLength, THRESHOLD);
			SDL_RenderCopy(gSDLRenderer, tex, nullptr, nullptr);
			SDL_RenderPresent(gSDLRenderer);
			RLOG(std::setprecision(10) << "origin: (" << originX << ", " << originY << ") screen width: " << pixelLength * WIDTH);
			shouldRender = false;
		}
	}

	SDL_DestroyTexture(tex);
	quit();
	return 0;
}