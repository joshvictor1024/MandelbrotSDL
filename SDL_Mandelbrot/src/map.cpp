#include "map.h"

Map::Map(int uSize, int vSize, Number_t texelLength) :
    uSize(uSize), vSize(vSize), texelLength(texelLength), chunkLength(texelLength* CHUNK_SIZE)
{
    texture = SDL_CreateTexture(SDLManager::renderer(), SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, uSize * CHUNK_SIZE, vSize * CHUNK_SIZE);

    RLOG("Texture size: " << uSize * CHUNK_SIZE << " * " << vSize * CHUNK_SIZE);

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
    RLOG("Range: (" << originX << ", " << originY << ") through (" << originX + width << ", " << originY - height << ")");

    updateBuffer();
}

void Map::updateBuffer()
{
    buffer.debugPrint();

    // Calculate new buffer based on current buffer and most recent range

    BufferChunks newBuffer;

    int chunkDu = floorDivide(range.x - buffer.x, chunkLength);
    int chunkDv = floorDivide(buffer.y - range.y, chunkLength);
    //RLOG("Chunk Difference: (" <<
    //    chunkDu << ", " <<
    //    chunkDv << ")");

    newBuffer.x = buffer.x + chunkDu * chunkLength;
    newBuffer.y = buffer.y - chunkDv * chunkLength;

    // Modulo later
    newBuffer.u = buffer.u + chunkDu;
    newBuffer.v = buffer.v + chunkDv;

    newBuffer.uSize = 1 - chunkDu + floorDivide(range.x + range.width - buffer.x, chunkLength);
    newBuffer.vSize = 1 - chunkDv + floorDivide(buffer.y - (range.y - range.height), chunkLength);
    //RLOG("Chunk wh: (" <<
    //    newBuffer.uSize << ", " <<
    //    newBuffer.vSize << ")");

    // Check chunks in newBuffer, if they need to be re-computed

    for (int v = newBuffer.v; v < newBuffer.v + newBuffer.vSize; ++v)
    {
        for (int u = newBuffer.u; u < newBuffer.u + newBuffer.uSize; ++u)
        {
            int uMod = floorModulo(u, uSize);
            int vMod = floorModulo(v, vSize);

            //RLOG("Checking chunk: (" << u << ", " << v << ")");

            if (
                ((u >= buffer.u && u < buffer.u + buffer.uSize) &&
                (v >= buffer.v && v < buffer.v + buffer.vSize)) == false
                )
            {
                RLOG("Mark to be re-computed, chunk: (" << u << ", " << v << ") i.e. (" << uMod << ", " << vMod << ") after floorModulo");
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
                //RLOG("Chunk: (" << uMod << ", " << vMod << ")");

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

// TODO: take pixelLength
void Map::renderCopy(SDL_Texture* screen)
{
    Number_t x = textureRightBorder();
    Number_t y = textureBottomBorder();
    RLOG("Border: (" << x << ", " << y << ")");

    SDL_SetRenderTarget(SDLManager::renderer(), screen);

    // Can go beyond border of texture
    SDL_Rect src = {
            (buffer.u * CHUNK_SIZE + (int)((range.x - buffer.x) / texelLength)) % (uSize * CHUNK_SIZE),
            (buffer.v * CHUNK_SIZE - (int)((range.y - buffer.y) / texelLength)) % (vSize * CHUNK_SIZE),
            range.width / texelLength,
            range.height / texelLength
    };
    RLOG(src.x << ", " << src.y << ", " << src.w << ", " << src.h);

    // Topleft of screen, bottomright on the texture
    SDL_Rect topleft = {
            src.x,
            src.y,
            uSize * CHUNK_SIZE - src.x,
            vSize * CHUNK_SIZE - src.y
    };
    {
        SDL_FRect dst = { 0, 0, topleft.w, topleft.h };

        SDL_RenderCopyF(SDLManager::renderer(), texture, &topleft, &dst);
    }

    // Topright of screen, bottomleft on the texture
    {
        SDL_Rect topright = {
            0,
            src.y,
            src.w - topleft.w,
            topleft.h
        };
        SDL_FRect dst = { topleft.w, 0, topright.w, topright.h };

        SDL_RenderCopyF(SDLManager::renderer(), texture, &topright, &dst);
    }

    // Bottomleft of screen, topright on the texture
    {
        SDL_Rect bottomleft = {
            src.x,
            0,
            topleft.w,
            src.h - topleft.h
        };
        SDL_FRect dst = { 0, topleft.h, bottomleft.w, bottomleft.h };

        SDL_RenderCopyF(SDLManager::renderer(), texture, &bottomleft, &dst);
    }

    // Bottomright of screen, topleft on the texture
    {
        SDL_Rect bottomright = {
            0,
            0,
            src.w - topleft.w,
            src.h - topleft.h
        };
        SDL_FRect dst = { topleft.w, topleft.h, bottomright.w, bottomright.h };

        SDL_RenderCopyF(SDLManager::renderer(), texture, &bottomright, &dst);
    }

    SDL_SetRenderTarget(SDLManager::renderer(), nullptr);
}