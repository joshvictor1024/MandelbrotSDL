#include "chunk.h"

Chunk::Chunk()
{
    iterations.resize(CHUNK_SIZE * CHUNK_SIZE);
}

Chunk::~Chunk()
{

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