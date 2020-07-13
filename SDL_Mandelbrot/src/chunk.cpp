#include "chunk.h"

typedef std::complex<Number_t> Complex_t;//TODO: not necessary?

Chunk::Chunk()
{
    iterations.resize(CHUNK_SIZE * CHUNK_SIZE);
}

Chunk::~Chunk()
{

}

// Compute ///////////////////////////////////////////////////////

constexpr Number_t absSquared(const Complex_t& c)
{
    return (c.real() * c.real() + c.imag() * c.imag());
}

void Chunk::compute(Number_t originX, Number_t originY, Number_t texelLength, Iteration_t threshold)
{
    int pos = 0;

    for (int texelY = 0; texelY < CHUNK_SIZE; ++texelY)
    {
        for (int texelX = 0; texelX < CHUNK_SIZE; ++texelX, ++pos)
        {
            Complex_t c = { 0.0, 0.0 };
            Complex_t dc =
            {
                originX + texelX * texelLength,
                originY - texelY * texelLength
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

            iterations[pos] = it;
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

inline void color(uint8_t* pixels, int pos, Iteration_t it, Iteration_t threshold)
{
    if (it == threshold)
    {
        pixels[pos + PIXELDATA_OFFSETR] = BOUNDED_COLOR.r * 0xff;
        pixels[pos + PIXELDATA_OFFSETG] = BOUNDED_COLOR.g * 0xff;
        pixels[pos + PIXELDATA_OFFSETB] = BOUNDED_COLOR.b * 0xff;
    }
    else
    {
        Color color = getInterpolated(it % COLOR_COUNT);

        pixels[pos + PIXELDATA_OFFSETR] = color.r * 0xff;
        pixels[pos + PIXELDATA_OFFSETG] = color.g * 0xff;
        pixels[pos + PIXELDATA_OFFSETB] = color.b * 0xff;
    }
}

void Chunk::draw(SDL_Texture* texture, Chunk_t chunkCoordX, Chunk_t chunkCoordY, Chunk_t mapWidth, Iteration_t threshold)
{
    PixelData_t* pixelData;
    int pitchDiscarded;

    SDL_LockTexture(texture, nullptr, (void**)(&pixelData), &pitchDiscarded);

    // (chunkCoordY * mapWidth * CHUNK_SIZE * CHUNK_SIZE) + (chunkCoordX * CHUNK_SIZE)
    int pos = (chunkCoordY * mapWidth * CHUNK_SIZE + chunkCoordX) * CHUNK_SIZE;

    // TODO: check that pixeldata index doesn't overflow
    for (int texelY = 0; texelY < CHUNK_SIZE; ++texelY, pos += mapWidth * CHUNK_SIZE)
    {
        for (int texelX = 0; texelX < CHUNK_SIZE; ++texelX)
        {
            color(pixelData, (pos + texelX) * TEXTURE_PITCH, iterations[texelY * CHUNK_SIZE + texelX], threshold);
        }
    }

    SDL_UnlockTexture(texture);
}