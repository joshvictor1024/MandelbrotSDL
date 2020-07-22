#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include "constants.h"
#include "sdl_manager.h"

// Wrapper around POD std::array, plus member functions
class Chunk
{
public:

    // Non-locking
    // Works with external lock or lock-free queue
    void Compute(Number_t originX, Number_t originY, Number_t texelLength, Iteration_t threshold);

    // Locking
    void Draw(SDL_Texture* texture, Chunk_t chunkUMod, Chunk_t ChunkVMod, Chunk_t mapUSize, Iteration_t threshold);

    constexpr static int SIZE = 256;    // in texels

    typedef uint8_t Status_t;
    constexpr static Status_t SHOULD_COMPUTE_BIT = 0x1;
    constexpr static Status_t SHOULD_DRAW_BIT = 0x2;
    constexpr static Status_t INIT = SHOULD_COMPUTE_BIT;

private:

    std::array<Iteration_t, SIZE * SIZE> iterations;
};

#endif // !CHUNK_H
