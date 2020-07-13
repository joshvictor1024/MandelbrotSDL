#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include "constants.h"
#include "sdl_manager.h"

constexpr int CHUNK_SIZE = 256;

class Chunk
{
public:

    Chunk();
    ~Chunk();

    void compute(Number_t originX, Number_t originY, Number_t texelLength, Iteration_t threshold);
    void draw(SDL_Texture* texture, Chunk_t chunkCoordX, Chunk_t ChunkCoordY, Chunk_t mapWidth, Iteration_t threshold);

    typedef uint8_t Status_t;
    static constexpr Status_t SHOULD_COMPUTE_BIT    = 0x1;
    static constexpr Status_t SHOULD_DRAW_BIT       = 0x2;

    static constexpr Status_t INIT                  = 0x0;
    static constexpr Status_t START_COMPUTE         = 0x0;
    static constexpr Status_t END_COMPUTE           = SHOULD_DRAW_BIT;
    static constexpr Status_t START_DRAW            = 0x0;

private:

    std::vector<Iteration_t> iterations;
};

#endif // !CHUNK_H
