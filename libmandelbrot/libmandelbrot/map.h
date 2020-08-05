#ifndef MAP_H
#define MAP_H

#include <vector>
#include <atomic>
#include "common.h"
#include "graphics.h"
#include "chunk.h"

namespace mdb {

// The smallest set of chunks that encompasses the screen
struct BufferChunks
{
    Number_t x = 0;
    Number_t y = 0;
    Chunk_t u = 0;
    Chunk_t v = 0;
    Chunk_t uSize = 0;
    Chunk_t vSize = 0;

    void debugPrint();
};

// Visible number range
struct NumberRange
{
    Number_t x = 0;
    Number_t y = 0;
    Number_t width = 0;
    Number_t height = 0;
};

// 2D circular buffer for iteration data, consisting of Chunks and their states
class Map
{
public:

    Map(Number_t texelLength, Chunk_t uSize, Chunk_t vSize) :
        chunks(std::vector<std::vector<Chunk>>(vSize, std::vector<Chunk>(uSize))),
        chunksStatus(std::vector<std::vector<Chunk::Status_t>>(vSize, std::vector<Chunk::Status_t>(uSize, Chunk::INIT))),
        texelLength(texelLength),
        chunkLength(texelLength * Chunk::SIZE),
        uSize(uSize),
        vSize(vSize) {}

    ~Map();

    /***************************************************************
        Controls
    ***************************************************************/
    
    // Stateful, based on current buffer aside from given range
    void UpdateBuffer(NumberRange range);

    // Dispatches work
    void UpdateState(std::unique_ptr<Texture>& texture, Iteration_t threshold);

    void Draw(std::unique_ptr<Texture>& source, NumberRange range, Number_t pixelLength);

    void Recompute()
    {
        chunksStatus = std::vector<std::vector<Chunk::Status_t>>(vSize, std::vector<Chunk::Status_t>(uSize, Chunk::INIT));
    }

    /***************************************************************
        texelLength
    ***************************************************************/

    [[nodiscard]] constexpr Number_t TexelLength() const noexcept { return texelLength; }

    void ChangeTexelLength(Number_t texelLength)
    {
        this->texelLength = texelLength;
        this->chunkLength = texelLength * Chunk::SIZE;
        Recompute();
    }

    constexpr static Number_t MIN_TEXEL_PER_PIXEL = 1;

    // In turn defines the smallest range i.e. closest zoom
    [[nodiscard]] constexpr Number_t MinPixelLength() const noexcept
    {
        return texelLength;
    }

    // In turn defines the largest range
    [[nodiscard]] constexpr Number_t MaxPixelLength(int screenWidth, int screenHeight) const
    {
        Number_t pixelLengthW = (Number_t)((uSize - 1) * Chunk::SIZE) / (Number_t)(screenWidth) * texelLength;
        Number_t pixelLengthH = (Number_t)((vSize - 1) * Chunk::SIZE) / (Number_t)(screenHeight) * texelLength;
        return (pixelLengthW < pixelLengthH) ? pixelLengthW : pixelLengthH;
    }

    [[nodiscard]] constexpr Number_t NextSmallerTexelLength(int screenWidth, int screenHeight) const
    {
        return TexelLengthFromMax(MinPixelLength(), screenWidth, screenHeight);
    }

    [[nodiscard]] constexpr Number_t PrevLargerTexelLength(int screenWidth, int screenHeight) const
    {
        return TexelLengthFromMin(MaxPixelLength(screenWidth, screenHeight));
    }
    
private:

    [[nodiscard]] constexpr Number_t TexelLengthFromMin(Number_t minPixelLength) const noexcept
    {
        return minPixelLength;
    }

    [[nodiscard]] constexpr Number_t TexelLengthFromMax(Number_t maxPixelLength, int screenWidth, int screenHeight) const
    {
        Number_t texelLengthW = (Number_t)(screenWidth) / (Number_t)((uSize - 1) * Chunk::SIZE) * maxPixelLength;
        Number_t texelLengthH = (Number_t)(screenHeight) / (Number_t)((vSize - 1) * Chunk::SIZE) * maxPixelLength;
        return (texelLengthW > texelLengthH) ? texelLengthW : texelLengthH;
    }

    /***************************************************************
        Utility
    ***************************************************************/

    // Return the represented number value on the right border
    [[nodiscard]] constexpr Number_t Right() const noexcept
    {
        return buffer.x + (uSize - buffer.u) * chunkLength;
    }

    // Return the represented number value on the bottom border
    [[nodiscard]] constexpr Number_t Bottom() const noexcept
    {
        return buffer.y - (vSize - buffer.v) * chunkLength;
    }

    std::vector<std::vector<Chunk>> chunks;
    std::vector<std::vector<Chunk::Status_t>> chunksStatus;
    BufferChunks buffer;
    Number_t texelLength;
    Number_t chunkLength;   // texelLength * Chunk::SIZE is commonly used
    Chunk_t uSize;
    Chunk_t vSize;

    static std::atomic<int> nowComputing;
};

} // namespace mdb

#endif // !MAP_H