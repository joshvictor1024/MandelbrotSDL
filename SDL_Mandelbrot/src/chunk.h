#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include "constants.h"
#include "sdl_manager.h"

constexpr int CHUNK_SIZE = 256;

typedef uint8_t ChunkStatus_t;
constexpr ChunkStatus_t CHUNK_SHOULD_COMPUTE_BIT = 1 << 0;
constexpr ChunkStatus_t CHUNK_DONE_COMPUTE_BIT = 1 << 1;
constexpr ChunkStatus_t CHUNK_SHOULD_DRAW_BIT = 1 << 2;
constexpr ChunkStatus_t CHUNK_DONE_DRAW_BIT = 1 << 3;
constexpr ChunkStatus_t CHUNK_INIT = CHUNK_SHOULD_COMPUTE_BIT;

//constexpr Position_t absSquared(const Complex_t& c) // TODO: proper location
//{
//    return (c.real() * c.real() + c.imag() + c.imag());
//}

class Chunk
{
public:

    Chunk();
    ~Chunk();

    void compute(Position_t originX, Position_t originY, Position_t texelLength, Iteration_t threshold);
    void draw(SDL_Texture* texture, int chunkCoordX, int ChunkCoordY, int mapWidth);

private:

    std::vector<Iteration_t> iterations;
};

Chunk::Chunk()
{
    iterations.resize(CHUNK_SIZE * CHUNK_SIZE);
}

Chunk::~Chunk()
{

}

void Chunk::compute(Position_t originX, Position_t originY, Position_t texelLength, Iteration_t threshold)
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

                if (absSquared(c) > static_cast<Position_t>(2 * 2))
                {
                    break;
                }
            }

            iterations[pos] = it;
        }
    }
}

void Chunk::draw(SDL_Texture* texture, int chunkCoordX, int chunkCoordY, int mapWidth)
{
    PixelData_t* pixelData;
    int pitchDiscarded;

    SDL_LockTexture(texture, nullptr, (void**)(&pixelData), &pitchDiscarded);

    // (chunkCoordY * mapWidth * CHUNK_SIZE * CHUNK_SIZE) + (chunkCoordX * CHUNK_SIZE)
    int pos = (chunkCoordY * mapWidth * CHUNK_SIZE + chunkCoordX) * CHUNK_SIZE;

    for (int texelY = 0; texelY < CHUNK_SIZE; ++texelY, pos += mapWidth * CHUNK_SIZE)
    {
        for (int texelX = 0; texelX < CHUNK_SIZE; ++texelX)
        {
            color(pixelData, (pos + texelX) * RGB888_SIZE, iterations[texelY * CHUNK_SIZE + texelX]);
        }
    }

    SDL_UnlockTexture(texture);
}

#endif // !CHUNK_H
