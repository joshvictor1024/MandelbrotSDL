#ifndef MAP_H
#define MAP_H

#include <vector>
#include <limits>
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
constexpr Chunk_t floorModulo(Chunk_t dividend, Chunk_t modulo)
{
    if (dividend >= 0)
    {
        return dividend % modulo;
    }
    else
    {
        return (dividend + 1) % modulo + modulo - 1;
    }
}

// Does modulo and take the positive remainder
// Corrects modulo for negative numbers
// Assume positive modulo
template<>
constexpr Number_t floorModulo(Number_t dividend, Number_t modulo)
{
    return dividend - floorDivide(dividend, modulo) * modulo;
}

// Class Declaration ///////////////////////////////////////////////////////

// Maintains 1) a 2D circular buffer for iteration data 2) a texture to draw to
template<Chunk_t U_SIZE, Chunk_t V_SIZE>
class Map
{
public:

    Map(Number_t texelLength);
    ~Map();

    void SetNumberRange(Number_t originX, Number_t originY, Number_t width, Number_t height);
    void Update(Iteration_t threshold);
    void RenderCopy(SDL_Texture* screen, Number_t pixelLength);

    void debugCopy()
    {
        SDL_Rect dst = { 0, 0, U_SIZE * 20, V_SIZE * 20 };
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
    Number_t TextureRight()
    {
        return buffer.x + (U_SIZE - buffer.u) * chunkLength;
    }

    // Return the represented number value
    Number_t TextureBottom()
    {
        return buffer.y - (V_SIZE - buffer.v) * chunkLength;
    }

    // Stateful, based on current buffer and most recent range
    void UpdateBuffer();

    SDL_Texture* texture = nullptr;

    std::vector<std::vector<Chunk>> chunks
        = std::vector<std::vector<Chunk>>(V_SIZE, std::vector<Chunk>(U_SIZE));

    std::vector<std::vector<Chunk::Status_t>> chunksStatus
        = std::vector<std::vector<Chunk::Status_t>>(V_SIZE, std::vector<Chunk::Status_t>(U_SIZE, Chunk::INIT));

    Number_t texelLength;
    Number_t chunkLength;   // texelLength * Chunk::SIZE is commonly used
    BufferChunks buffer;
    NumberRange range;
};

// Methods ///////////////////////////////////////////////////////

template<Chunk_t U_SIZE, Chunk_t V_SIZE>
Map<U_SIZE, V_SIZE>::Map(Number_t texelLength) :
    texelLength(texelLength), chunkLength(texelLength* Chunk::SIZE)
{
    ILOG("Texture size: " << U_SIZE * Chunk::SIZE << " * " << V_SIZE * Chunk::SIZE);
    DLOG("Texture texel count: " << U_SIZE * V_SIZE * Chunk::SIZE * Chunk::SIZE);
    DLOG("PixelDataIndex_t max value: " << std::numeric_limits<PixelDataIndex_t>::max());

    static_assert
    (
        U_SIZE * V_SIZE * Chunk::SIZE * Chunk::SIZE <= std::numeric_limits<PixelDataIndex_t>::max(),
        "PixelDataIndex_t will overflow, texture is too large. Reduce Map U_SIZE/V_SIZE or Chunk::SIZE."
    );

    texture = SDL_CreateTexture
    (
        SDLManager::renderer(), SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
        U_SIZE * Chunk::SIZE, V_SIZE * Chunk::SIZE
    );
}

template<Chunk_t U_SIZE, Chunk_t V_SIZE>
Map<U_SIZE, V_SIZE>::~Map()
{
    SDL_DestroyTexture(texture);
}

template<Chunk_t U_SIZE, Chunk_t V_SIZE>
void Map<U_SIZE, V_SIZE>::SetNumberRange(Number_t originX, Number_t originY, Number_t width, Number_t height)
{
    range = { originX, originY, width, height };
    DLOG("Range: (" << originX << ", " << originY << ") through (" << originX + width << ", " << originY - height << ")");

    UpdateBuffer();
}

template<Chunk_t U_SIZE, Chunk_t V_SIZE>
void Map<U_SIZE, V_SIZE>::UpdateBuffer()
{
    buffer.debugPrint();

    // Calculate new buffer based on current buffer and most recent range

    BufferChunks newBuffer;

    // Using floating point to avoid overflow, accepting floating point error
    // Check overflow later when storing value back in Chunk_t
    Number_t chunkDu = floorDivide(range.x - buffer.x, chunkLength);
    Number_t chunkDv = floorDivide(buffer.y - range.y, chunkLength);
    //ILOG("Chunk Difference: (" <<
    //    chunkDu << ", " <<
    //    chunkDv << ")");

    newBuffer.x = buffer.x + chunkDu * chunkLength;
    newBuffer.y = buffer.y - chunkDv * chunkLength;

    newBuffer.uSize = 1 + floorDivide(range.x + range.width - newBuffer.x, chunkLength);
    newBuffer.vSize = 1 + floorDivide(newBuffer.y - (range.y - range.height), chunkLength);
    //ILOG("Chunk wh: (" <<
    //    newBuffer.uSize << ", " <<
    //    newBuffer.vSize << ")");

    // If u or v is more than uSize or vSize off, then all chunks need re-computing
    // No need to check individual chunks then
    // Also, prevents new u, v from overflowing Chunk_t

    if (
        chunkDu > U_SIZE || chunkDu < -1 * U_SIZE ||
        chunkDv > V_SIZE || chunkDv < -1 * V_SIZE
        )
    {
        DLOG("All chunks in bound need to be computed!");

        // Fitting new u, v to old buffer

        Chunk_t chunkDuMod = floorModulo(chunkDu, static_cast<Number_t>(U_SIZE));
        Chunk_t chunkDvMod = floorModulo(chunkDv, static_cast<Number_t>(V_SIZE));

        buffer.u = floorModulo(static_cast<Chunk_t>(buffer.u + chunkDuMod), U_SIZE);
        buffer.v = floorModulo(static_cast<Chunk_t>(buffer.v + chunkDvMod), V_SIZE);

        // Other fields can just be copied

        buffer.x = newBuffer.x;
        buffer.y = newBuffer.y;

        buffer.uSize = newBuffer.uSize;
        buffer.vSize = newBuffer.vSize;

        // Set all bounded chunks to be computed

        for (Chunk_t v = buffer.v; v < buffer.v + buffer.vSize; ++v)
        {
            for (Chunk_t u = buffer.u; u < buffer.u + buffer.uSize; ++u)
            {
                // floorModulo not needed: stays positive and within +1 modulo
                Chunk_t uMod = u % U_SIZE;
                Chunk_t vMod = v % V_SIZE;

                DLOG("Mark to be re-computed, chunk: (" << u << ", " << v << ") i.e. (" << uMod << ", " << vMod << ") after floorModulo");
                chunksStatus[vMod][uMod] |= Chunk::SHOULD_COMPUTE_BIT;
            }
        }
    }
    else
    {
        DLOG("Not all chunks in bound need to be computed.");

        // Modulo later when copying data back to buffer
        // Non-modulo value needed to check bounds

        newBuffer.u = buffer.u + chunkDu;
        newBuffer.v = buffer.v + chunkDv;

        // Check chunks in new bound (newBuffer), if they need to be re-computed

        for (Chunk_t v = newBuffer.v; v < newBuffer.v + newBuffer.vSize; ++v)
        {
            for (Chunk_t u = newBuffer.u; u < newBuffer.u + newBuffer.uSize; ++u)
            {
                Chunk_t uMod = floorModulo(u, U_SIZE);
                Chunk_t vMod = floorModulo(v, V_SIZE);

                // If the chunks are out of the original bound
                //ILOG("Checking chunk: (" << u << ", " << v << ")");
                if (
                    (u >= buffer.u && u < buffer.u + buffer.uSize &&
                        v >= buffer.v && v < buffer.v + buffer.vSize) == false
                    )
                {
                    DLOG("Mark to be re-computed, chunk: (" << u << ", " << v << ") i.e. (" << uMod << ", " << vMod << ") after floorModulo");
                    chunksStatus[vMod][uMod] |= Chunk::SHOULD_COMPUTE_BIT;
                }
            }
        }

        // Storing data of new buffer, doing floorModulo() to u, v
        // And ditching old buffer

        buffer.x = newBuffer.x;
        buffer.y = newBuffer.y;
        buffer.u = floorModulo(newBuffer.u, U_SIZE);
        buffer.v = floorModulo(newBuffer.v, V_SIZE);
        buffer.uSize = newBuffer.uSize;
        buffer.vSize = newBuffer.vSize;
    }

    buffer.debugPrint();
}

template<Chunk_t U_SIZE, Chunk_t V_SIZE>
void Map<U_SIZE, V_SIZE>::Update(Iteration_t threshold)
{
    for (Chunk_t v = buffer.v; v < buffer.v + buffer.vSize; ++v)
    {
        for (Chunk_t u = buffer.u; u < buffer.u + buffer.uSize; ++u)
        {
            // floorModulo not needed: stays positive and within +1 modulo
            Chunk_t uMod = u % U_SIZE;
            Chunk_t vMod = v % V_SIZE;
            Chunk::Status_t& status = chunksStatus[vMod][uMod];
            Chunk& chunk = chunks[vMod][uMod];

            if (status & Chunk::SHOULD_COMPUTE_BIT)
            {
                //ILOG("Chunk: (" << uMod << ", " << vMod << ")");

                status &= ~Chunk::SHOULD_COMPUTE_BIT;

                chunk.Compute(
                    buffer.x + (u - buffer.u) * chunkLength,
                    buffer.y - (v - buffer.v) * chunkLength,
                    texelLength,
                    threshold
                );

                status |= Chunk::SHOULD_DRAW_BIT;
            }
            else if (status & Chunk::SHOULD_DRAW_BIT)
            {
                status &= ~Chunk::SHOULD_DRAW_BIT;

                chunk.Draw(texture, uMod, vMod, U_SIZE, threshold);
            }
        }
    }
}

template<Chunk_t U_SIZE, Chunk_t V_SIZE>
void Map<U_SIZE, V_SIZE>::RenderCopy(SDL_Texture* screen, Number_t pixelLength)
{
    Number_t x = TextureRight();
    Number_t y = TextureBottom();
    DLOG("Border: (" << x << ", " << y << ")");

    Number_t texelPerPixel = texelLength / pixelLength;

    // Can go beyond border of texture
    // floorModulo not needed: stays positive and within +1 modulo
    SDL_Rect src = {
            (buffer.u * Chunk::SIZE + (int)((range.x - buffer.x) / texelLength)) % (U_SIZE * Chunk::SIZE),
            (buffer.v * Chunk::SIZE - (int)((range.y - buffer.y) / texelLength)) % (V_SIZE * Chunk::SIZE),
            range.width / pixelLength,
            range.height / pixelLength
    };
    DLOG("src: " << src.x << ", " << src.y << ", " << src.w << ", " << src.h);

    SDL_SetRenderTarget(SDLManager::renderer(), screen);

    // Topleft of screen, bottomright on the texture
    SDL_Rect topleft = {
            src.x,
            src.y,
            U_SIZE * Chunk::SIZE - src.x,
            V_SIZE * Chunk::SIZE - src.y
    };
    DLOG("topleft: " << topleft.x << ", " << topleft.y << ", " << topleft.w << ", " << topleft.h);
    {
        SDL_FRect dst = { 0, 0, topleft.w * texelPerPixel, topleft.h * texelPerPixel };
        DLOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

        SDL_RenderCopyF(SDLManager::renderer(), texture, &topleft, &dst);
    }

    // Topright of screen, bottomleft on the texture
    DLOG("if topleft.w ( " << topleft.w << " ) < range.width * texelLength ( " << range.width / texelLength << " )");
    if (topleft.w < range.width / texelLength)
    {
        SDL_Rect topright = {
            0,
            src.y,
            range.width / texelLength - topleft.w,
            topleft.h
        };
        SDL_FRect dst = { topleft.w * texelPerPixel, 0, topright.w * texelPerPixel, topright.h * texelPerPixel };
        DLOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

        SDL_RenderCopyF(SDLManager::renderer(), texture, &topright, &dst);
    }

    // Bottomleft of screen, topright on the texture
    DLOG("if topleft.h ( " << topleft.h << " ) < range.height * texelLength ( " << range.height / texelLength << " )");
    if (topleft.h < range.height / texelLength)
    {
        {
            SDL_Rect bottomleft = {
                src.x,
                0,
                topleft.w,
                range.height / texelLength - topleft.h
            };
            SDL_FRect dst = { 0, topleft.h * texelPerPixel, bottomleft.w * texelPerPixel, bottomleft.h * texelPerPixel };
            DLOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

            SDL_RenderCopyF(SDLManager::renderer(), texture, &bottomleft, &dst);
        }

        // Bottomright of screen, topleft on the texture
        {
            SDL_Rect bottomright = {
                0,
                0,
                range.width / texelLength - topleft.w,
                range.height / texelLength - topleft.h
            };
            SDL_FRect dst = { topleft.w * texelPerPixel, topleft.h * texelPerPixel, bottomright.w * texelPerPixel, bottomright.h * texelPerPixel };
            DLOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

            SDL_RenderCopyF(SDLManager::renderer(), texture, &bottomright, &dst);
        }
    }

    SDL_SetRenderTarget(SDLManager::renderer(), nullptr);
}

#endif // !MAP_H