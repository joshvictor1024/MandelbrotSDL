#ifndef MAP_H
#define MAP_H

#include <vector>
#include "constants.h"
#include "sdl_manager.h"
#include "chunk.h"

// 2D circular buffer for iteration data, and a texture to draw to
class Map
{
public:

    Map(Chunk_t uSize, Chunk_t vSize, Number_t texelLength);
    ~Map();

    void setNumberRange(Number_t originX, Number_t originY, Number_t width, Number_t height);
    void update(Iteration_t threshold);
    void renderCopy(SDL_Texture* screen, Number_t pixelLength);
    void debugCopy()
    {
        SDL_Rect dst = { 0, 0, uSize * 20, vSize * 20 };
        SDL_RenderCopy(SDLManager::renderer(), texture, nullptr, &dst);
    }

private:

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

    // Return the represented number value 
    Number_t textureRightBorder()
    {
        return buffer.x + (uSize - buffer.u) * chunkLength;
    }

    // Return the represented number value
    Number_t textureBottomBorder()
    {
        return buffer.y - (vSize - buffer.v) * chunkLength;
    }

    // Stateful, based on current buffer and most recent range
    void updateBuffer();

    SDL_Texture* texture = nullptr;
    std::vector<std::vector<Chunk>> chunks;
    std::vector<std::vector<Chunk::Status_t>> chunksStatus;
    Chunk_t uSize;  // In chunks
    Chunk_t vSize;  // In chunks
    
    Number_t texelLength;
    Number_t chunkLength;   // texelLength * CHUNK_SIZE is commonly used
    BufferChunks buffer;
    NumberRange range;
};

#endif // !MAP_H
