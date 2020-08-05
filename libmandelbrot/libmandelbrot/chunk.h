#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include "common.h"
#include "graphics.h"

namespace mdb {

// Wrapper around POD std::array, plus member functions
class Chunk
{
public:

    // Writes to owning memory
    // Non-locking
    void Compute(Number_t originX, Number_t originY, Number_t texelLength, Iteration_t threshold);

    // Writes to non-owning memory
    // Consider external locking
    void Draw(std::unique_ptr<Texture>& texture, Chunk_t chunkUMod, Chunk_t ChunkVMod, Iteration_t threshold);

    constexpr static int SIZE = 256;    // in texels

    typedef uint8_t Status_t;
    constexpr static Status_t SHOULD_COMPUTE_BIT = 0x1;
    constexpr static Status_t SHOULD_DRAW_BIT = 0x2;
    constexpr static Status_t INIT = SHOULD_COMPUTE_BIT;

private:

    std::array<Iteration_t, SIZE * SIZE> iterations;
};

} // namespace mdb

#endif // !CHUNK_H
