#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <complex>

// Types ///////////////////////////////////////////////////////

typedef double      Number_t;
typedef uint16_t    Iteration_t;
typedef int16_t     Chunk_t;        // Could technically use int8_t, but char does't play nice with text output
typedef uint8_t     PixelData_t;
typedef uint32_t    PixelDataIndex_t;

// Window ///////////////////////////////////////////////////////

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

// Graphics ///////////////////////////////////////////////////////

constexpr int MOVEPIXEL_PER_MS = 200;

constexpr int MAX_FRAMERATE = 50;
constexpr int MIN_FRAMETIME = 1000 / MAX_FRAMERATE;

constexpr Number_t DEFAULT_X = -2.4;
constexpr Number_t DEFAULT_Y = 1.075;
constexpr Number_t DEFAULT_PIXEL_LENGTH = 0.003;
constexpr Iteration_t DEFAULT_THRESHOLD = 256;

// Deprecated
constexpr float SCROLL_ZOOM_OUT = 1.0f / 0.8f;

constexpr float SCROLL_ZOOM_DIFFERENCE = 0.25f;

constexpr float MAX_PIXEL_LENGTH = 0.005f;

// Texture ///////////////////////////////////////////////////////

// TODO: separate Texture (and event, ... etc. so as not to be locked to SDL)

//constexpr auto TEXTURE_PIXELFORMAT = SDL_PIXELFORMAT_RGB888;
// https://stackoverflow.com/questions/29061505/where-can-i-get-correct-pitch-parameter-for-sdl-renderreadpixels-function-in-sdl
// https://github.com/spurious/SDL-mirror/blob/master/src/video/SDL_surface.c
// Taken from SDL_CalculatePitch
constexpr int TEXTURE_PITCH = 4;

constexpr int PIXELDATA_OFFSETR = 2;    // For the selected pixelformat
constexpr int PIXELDATA_OFFSETG = 1;    // For the selected pixelformat
constexpr int PIXELDATA_OFFSETB = 0;    // For the selected pixelformat

//#include "boost\multiprecision\cpp_bin_float.hpp"
//#include "boost\multiprecision\cpp_complex.hpp"

//constexpr int PRECISION = 25;
//typedef boost::multiprecision::number<boost::multiprecision::backends::cpp_bin_float<PRECISION>> Number_t;
//typedef boost::multiprecision::cpp_complex<PRECISION> Complex_t;

#endif // !CONSTANTS_H
