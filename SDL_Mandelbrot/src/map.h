#ifndef MAP_H
#define MAP_H

#include <vector>
#include "constants.h"
#include "sdl_manager.h"
#include "chunk.h"

// The smallest set of chunks that encompasses the screen
struct BufferChunks
{
    Number_t x = 0;
    Number_t y = 0;
    Chunk_t u = 0;
    Chunk_t v = 0;
    Chunk_t uSize = 0;
    Chunk_t vSize = 0;

    void debugPrint()
    {
        DLOG("Number (x, y): (" <<
            x << ", " <<
            y << ")");
        DLOG("Chunks: (" <<
            u << ", " <<
            v << ") inclusive through (" <<
            u + uSize << ", " <<
            v + vSize << ") exclusive");
    }
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
    Map(Number_t texelLength, Map&& other) :
        chunks(std::move(other.chunks)),
        chunksStatus(std::vector<std::vector<Chunk::Status_t>>(other.vSize, std::vector<Chunk::Status_t>(other.uSize, Chunk::INIT))),
        texelLength(texelLength),
        chunkLength(texelLength* Chunk::SIZE),
        uSize(other.uSize),
        vSize(other.vSize)
    {
        // TODO: resize chunks if needed
    }
    
    // Stateful, based on current buffer aside from given range
    void UpdateBuffer(NumberRange range);

    // Dispatches work
    void UpdateState(SDL_Texture* texture, Iteration_t threshold);

    void RenderCopy(SDL_Texture* screen, SDL_Texture* source, NumberRange range, Number_t pixelLength);

    Number_t TexelLength() const noexcept { return texelLength; }

    constexpr static Number_t MIN_TEXEL_PER_PIXEL = 1;

    // In turn defines the smallest range i.e. closest zoom
    Number_t MinPixelLength() noexcept
    {
        return texelLength;
    }

    // In turn defines the largest range
    Number_t MaxPixelLength(int screenWidth, int screenHeight)
    {
        Number_t pixelLengthW = (Number_t)((uSize - 1) * Chunk::SIZE) / (Number_t)(screenWidth) * texelLength;
        Number_t pixelLengthH = (Number_t)((vSize - 1) * Chunk::SIZE) / (Number_t)(screenHeight) * texelLength;
        return (pixelLengthW < pixelLengthH) ? pixelLengthW : pixelLengthH;
    }

    Number_t NextSmallerTexelLength(int screenWidth, int screenHeight)
    {
        return TexelLengthFromMax(MinPixelLength(), screenWidth, screenHeight);
    }

    Number_t PrevLargerTexelLength(int screenWidth, int screenHeight)
    {
        return TexelLengthFromMin(MaxPixelLength(screenWidth, screenHeight));
    }
    
private:

    Number_t TexelLengthFromMin(Number_t minPixelLength) noexcept
    {
        return minPixelLength;
    }

    Number_t TexelLengthFromMax(Number_t maxPixelLength, int screenWidth, int screenHeight)
    {
        Number_t texelLengthW = (Number_t)(screenWidth) / (Number_t)((uSize - 1) * Chunk::SIZE) * maxPixelLength;
        Number_t texelLengthH = (Number_t)(screenHeight) / (Number_t)((vSize - 1) * Chunk::SIZE) * maxPixelLength;
        return (texelLengthW > texelLengthH) ? texelLengthW : texelLengthH;
    }

    // Return the represented number value on the right border
    Number_t Right() noexcept
    {
        return buffer.x + (uSize - buffer.u) * chunkLength;
    }

    // Return the represented number value on the bottom border
    Number_t Bottom() noexcept
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
};

#endif // !MAP_H