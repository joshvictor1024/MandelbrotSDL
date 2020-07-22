#include "chunk.h"

typedef std::complex<Number_t> Complex_t;//TODO: not necessary?

// Compute ///////////////////////////////////////////////////////

constexpr Number_t absSquared(const Complex_t& c) noexcept
{
    return (c.real() * c.real() + c.imag() * c.imag());
}

void Chunk::Compute(Number_t originX, Number_t originY, Number_t texelLength, Iteration_t threshold)
{
    int index = 0;

    for (int texelV = 0; texelV < SIZE; ++texelV)
    {
        for (int texelU = 0; texelU < SIZE; ++texelU, ++index)
        {
            Complex_t c = { 0.0, 0.0 };
            Complex_t dc =
            {
                originX + texelU * texelLength,
                originY - texelV * texelLength
            };

            Iteration_t it = 0;
            for (; it < threshold; ++it)
            {
                c = c * c + dc;

                if (absSquared(c) > static_cast<Number_t>(2 * 2))
                {
                    break;
                }
            }

            iterations[index] = it;
        }
    }
}

// Color ///////////////////////////////////////////////////////

struct Color
{
    float r;
    float g;
    float b;

    constexpr static int size = 3;
};

constexpr int PALETTE_SIZE = 16;
constexpr int INTERVAL = 16;
constexpr int COLOR_COUNT = INTERVAL * PALETTE_SIZE;

// Set draw color here
constexpr Color BOUNDED_COLOR = { 0.0f, 0.0f, 0.0f };
constexpr Color UNBOUNDED_COLORS[PALETTE_SIZE + 1] =
{
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f },

    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.5f, 0.0f },
    { 0.0f, 0.0f, 0.0f },

    { 1.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.5f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },

    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f },

    { 0.0f, 0.0f, 1.0f }    // repeats the first color
};

constexpr Color getInterpolated(int iterationsMod)
{
    int b = iterationsMod / INTERVAL;
    int m = iterationsMod % INTERVAL;

    return
    {
        UNBOUNDED_COLORS[b].r + (UNBOUNDED_COLORS[b + 1].r - UNBOUNDED_COLORS[b].r) * ((float)m / (float)INTERVAL),
        UNBOUNDED_COLORS[b].g + (UNBOUNDED_COLORS[b + 1].g - UNBOUNDED_COLORS[b].g) * ((float)m / (float)INTERVAL),
        UNBOUNDED_COLORS[b].b + (UNBOUNDED_COLORS[b + 1].b - UNBOUNDED_COLORS[b].b) * ((float)m / (float)INTERVAL)
    };
}

inline void color(uint8_t* pixels, int index, Iteration_t it, Iteration_t threshold)
{
    if (it == threshold)
    {
        pixels[index + PIXELDATA_OFFSETR] = BOUNDED_COLOR.r * 0xff;
        pixels[index + PIXELDATA_OFFSETG] = BOUNDED_COLOR.g * 0xff;
        pixels[index + PIXELDATA_OFFSETB] = BOUNDED_COLOR.b * 0xff;
    }
    else
    {
        Color color = getInterpolated(it % COLOR_COUNT);

        pixels[index + PIXELDATA_OFFSETR] = color.r * 0xff;
        pixels[index + PIXELDATA_OFFSETG] = color.g * 0xff;
        pixels[index + PIXELDATA_OFFSETB] = color.b * 0xff;
    }
}

void Chunk::Draw(SDL_Texture* texture, Chunk_t chunkUMod, Chunk_t chunkVMod, Chunk_t mapUSize, Iteration_t threshold)
{
    PixelData_t* pixelData;
    int pitchDiscarded;

    SDL_LockTexture(texture, nullptr, (void**)(&pixelData), &pitchDiscarded);

    // (chunkVMod * mapUSize * (SIZE * SIZE)) + (chunkUMod * SIZE)
    PixelDataIndex_t index = (chunkVMod * mapUSize * SIZE + chunkUMod) * SIZE;

    for (PixelDataIndex_t texelV = 0; texelV < SIZE; ++texelV, index += mapUSize * SIZE)
    {
        for (PixelDataIndex_t texelU = 0; texelU < SIZE; ++texelU)
        {
            color(pixelData, (index + texelU) * TEXTURE_PITCH, iterations[texelV * SIZE + texelU], threshold);
        }
    }

    SDL_UnlockTexture(texture);
}