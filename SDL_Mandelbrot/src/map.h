#ifndef MAP_H
#define MAP_H

#include <vector>
#include "constants.h"
#include "sdl_manager.h"
#include "chunk.h"

// Utility ///////////////////////////////////////////////////////

template<typename T>
constexpr int floorDivide(T dividend, T modulo);

// Integer divide and truncate towards negative infinity
// Corrects integer division for negative numbers
// Assume positive divisor
template<>
constexpr int floorDivide(Number_t dividend, Number_t divisor)
{
    if (dividend >= 0)
    {
        return (int)(dividend / divisor);
    }
    else
    {
        return (int)(dividend / divisor) - 1;
    }
}

// Integer divide and truncate towards negative infinity
// Corrects integer division for negative numbers
// Assume positive divisor
template<>
constexpr int floorDivide(int dividend, int divisor)
{
    if (dividend >= 0)
    {
        return dividend / divisor;
    }
    else
    {
        return dividend / divisor - 1;
    }
}

template<typename T>
constexpr T floorModulo(T dividend, T modulo);

// Does modulo and take the positive remainder
// Corrects modulo for negative numbers
// Assume positive modulo
template<>
constexpr int floorModulo(int dividend, int modulo)
{
    if (dividend >= 0)
    {
        return dividend % modulo;
    }
    else
    {
        return dividend % modulo + modulo;
    }
}

// Class ///////////////////////////////////////////////////////

class Map
{
public:

    Map(int uSize, int vSize, Number_t texelLength);
    ~Map();

    void setNumberRange(Number_t originX, Number_t originY, Number_t width, Number_t height);
    void update(Iteration_t threshold);
    //void renderCopy(SDL_Texture* screen);
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
        int u = 0;
        int v = 0;
        int uSize = 0;
        int vSize = 0;

        void debugPrint()
        {
            RLOG("Number (x, y): (" <<
                x << ", " <<
                y << ")");
            RLOG("Chunks: (" <<
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
    int uSize;  // In chunks
    int vSize;  // In chunks
    
    Number_t texelLength;
    Number_t chunkLength;   // texelLength * CHUNK_SIZE
    BufferChunks buffer;
    NumberRange range;
};

#endif // !MAP_H
