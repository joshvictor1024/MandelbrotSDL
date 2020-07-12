#include "map.h"

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

Map::Map(int uSize, int vSize, Number_t texelLength) :
    uSize(uSize), vSize(vSize), texelLength(texelLength), chunkLength(texelLength* CHUNK_SIZE)
{
    texture = SDL_CreateTexture(SDLManager::renderer(), SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, uSize * CHUNK_SIZE, vSize * CHUNK_SIZE);

    ILOG("Texture size: " << uSize * CHUNK_SIZE << " * " << vSize * CHUNK_SIZE);

    chunks.resize(vSize);
    for (auto& i : chunks)
    {
        i.resize(uSize);
    }

    chunksStatus.resize(vSize);
    for (auto& i : chunksStatus)
    {
        i.resize(uSize);

        for (auto& j : i)
        {
            j = Chunk::INIT;
        }
    }
}

Map::~Map()
{
    SDL_DestroyTexture(texture);
}

void Map::setNumberRange(Number_t originX, Number_t originY, Number_t width, Number_t height)
{
    range = { originX, originY, width, height };
    ILOG("Range: (" << originX << ", " << originY << ") through (" << originX + width << ", " << originY - height << ")");

    updateBuffer();
}

void Map::updateBuffer()
{
    buffer.debugPrint();

    // Calculate new buffer based on current buffer and most recent range

    BufferChunks newBuffer;

    int chunkDu = floorDivide(range.x - buffer.x, chunkLength);
    int chunkDv = floorDivide(buffer.y - range.y, chunkLength);
    //ILOG("Chunk Difference: (" <<
    //    chunkDu << ", " <<
    //    chunkDv << ")");

    newBuffer.x = buffer.x + chunkDu * chunkLength;
    newBuffer.y = buffer.y - chunkDv * chunkLength;

    // Modulo later
    newBuffer.u = buffer.u + chunkDu;
    newBuffer.v = buffer.v + chunkDv;

    newBuffer.uSize = 1 - chunkDu + floorDivide(range.x + range.width - buffer.x, chunkLength);
    newBuffer.vSize = 1 - chunkDv + floorDivide(buffer.y - (range.y - range.height), chunkLength);
    //ILOG("Chunk wh: (" <<
    //    newBuffer.uSize << ", " <<
    //    newBuffer.vSize << ")");

    // Check chunks in newBuffer, if they need to be re-computed

    for (int v = newBuffer.v; v < newBuffer.v + newBuffer.vSize; ++v)
    {
        for (int u = newBuffer.u; u < newBuffer.u + newBuffer.uSize; ++u)
        {
            int uMod = floorModulo(u, uSize);
            int vMod = floorModulo(v, vSize);

            //ILOG("Checking chunk: (" << u << ", " << v << ")");

            if (
                ((u >= buffer.u && u < buffer.u + buffer.uSize) &&
                (v >= buffer.v && v < buffer.v + buffer.vSize)) == false
                )
            {
                ILOG("Mark to be re-computed, chunk: (" << u << ", " << v << ") i.e. (" << uMod << ", " << vMod << ") after floorModulo");
                chunksStatus[vMod][uMod] |= Chunk::SHOULD_COMPUTE_BIT;
            }
        }
    }

    // Storing data of new buffer, doing floorModulo() to u, v
    // And ditching old buffer

    buffer.x = newBuffer.x;
    buffer.y = newBuffer.y;
    buffer.u = floorModulo(newBuffer.u, uSize);
    buffer.v = floorModulo(newBuffer.v, vSize);
    buffer.uSize = newBuffer.uSize;
    buffer.vSize = newBuffer.vSize;

    buffer.debugPrint();
}

void Map::update(Iteration_t threshold)
{
    for (int v = buffer.v; v < buffer.v + buffer.vSize; ++v)
    {
        for (int u = buffer.u; u < buffer.u + buffer.uSize; ++u)
        {
            int uMod = u % uSize;
            int vMod = v % vSize;
            Chunk::Status_t& status = chunksStatus[vMod][uMod];
            Chunk& chunk = chunks[vMod][uMod];

            if (status & Chunk::SHOULD_COMPUTE_BIT)
            {
                //ILOG("Chunk: (" << uMod << ", " << vMod << ")");

                status = Chunk::START_COMPUTE;

                chunk.compute(
                    buffer.x + ((uMod + uSize - buffer.u) % uSize) * chunkLength,
                    buffer.y - ((vMod + vSize - buffer.v) % vSize) * chunkLength,
                    texelLength,
                    threshold
                );

                status = Chunk::END_COMPUTE;
            }
            else if (status & Chunk::SHOULD_DRAW_BIT)
            {
                status = Chunk::START_DRAW;

                chunk.draw(texture, uMod, vMod, uSize);
            }
        }
    }
}

void Map::renderCopy(SDL_Texture* screen, Number_t pixelLength)
{
    Number_t x = textureRightBorder();
    Number_t y = textureBottomBorder();
    ILOG("Border: (" << x << ", " << y << ")");

    Number_t texelPerPixel = texelLength / pixelLength;

    // Can go beyond border of texture
    SDL_Rect src = {
            (buffer.u * CHUNK_SIZE + (int)((range.x - buffer.x) / texelLength)) % (uSize * CHUNK_SIZE),
            (buffer.v * CHUNK_SIZE - (int)((range.y - buffer.y) / texelLength)) % (vSize * CHUNK_SIZE),
            range.width / pixelLength,
            range.height / pixelLength
    };
    ILOG("src: " << src.x << ", " << src.y << ", " << src.w << ", " << src.h);

    SDL_SetRenderTarget(SDLManager::renderer(), screen);

    // Topleft of screen, bottomright on the texture
    SDL_Rect topleft = {
            src.x,
            src.y,
            uSize * CHUNK_SIZE - src.x,
            vSize * CHUNK_SIZE - src.y
    };
    ILOG("topleft: " << topleft.x << ", " << topleft.y << ", " << topleft.w << ", " << topleft.h);
    {
        SDL_FRect dst = { 0, 0, topleft.w * texelPerPixel, topleft.h * texelPerPixel };
        ILOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

        SDL_RenderCopyF(SDLManager::renderer(), texture, &topleft, &dst);
    }

    // Topright of screen, bottomleft on the texture
    ILOG("if topleft.w ( " << topleft.w << " ) < range.width * texelLength ( " << range.width / texelLength << " )");
    if (topleft.w < range.width / texelLength)
    {
        SDL_Rect topright = {
            0,
            src.y,
            range.width / texelLength - topleft.w,
            topleft.h
        };
        SDL_FRect dst = { topleft.w * texelPerPixel, 0, topright.w * texelPerPixel, topright.h * texelPerPixel };
        ILOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

        SDL_RenderCopyF(SDLManager::renderer(), texture, &topright, &dst);
    }

    // Bottomleft of screen, topright on the texture
    ILOG("if topleft.h ( " << topleft.h << " ) < range.height * texelLength ( " << range.height / texelLength << " )");
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
            ILOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

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
            ILOG("dst: " << dst.x << ", " << dst.y << ", " << dst.w << ", " << dst.h);

            SDL_RenderCopyF(SDLManager::renderer(), texture, &bottomright, &dst);
        }
    }

    SDL_SetRenderTarget(SDLManager::renderer(), nullptr);
}