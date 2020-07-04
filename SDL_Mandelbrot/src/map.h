#ifndef MAP_H
#define MAP_H

#include <vector>
#include "constants.h"
#include "sdl_manager.h"
#include "chunk.h"

class Map
{
public:

    Map(int width, int height, Position_t texelLength);
    ~Map();

    void setRange(Position_t originX, Position_t originY, Position_t width, Position_t height);
    void update(Iteration_t threshold);
    void renderCopy(SDL_Texture* screen);
    void debugCopy()
    {
        SDL_Rect dst = { 0, 0, width * 20, height * 20 };
        SDL_RenderCopy(SDLManager::renderer(), texture, nullptr, &dst);
    }

private:

    // The smallest set of chunks that encompasses the screen
    struct Buffer
    {
        Position_t x = 0;
        Position_t y = 0;
        int chunkX = 0;
        int chunkY = 0;
        int chunkWidth = 0;
        int chunkHeight = 0;
    };

    // Visible number range
    struct Range
    {
        Position_t x = 0;
        Position_t y = 0;
        Position_t width = 0;
        Position_t height = 0;
    };

    // Represented by the actual border of texture
    Position_t borderX()
    {
        return buffer.x + (width - buffer.chunkX) * CHUNK_SIZE * texelLength;
    }

    // Represented by the actual border of texture
    Position_t borderY()
    {
        return buffer.y - (height - buffer.chunkY) * CHUNK_SIZE * texelLength;
    }

    // From buffer.chunkX to the chunk x is in
    int chunkDifferenceX(Position_t x)
    {
        if (x > buffer.x)
        {
            return (int)((x - buffer.x) / (texelLength)) / CHUNK_SIZE;
        }
        else
        {
            return (int)((x - buffer.x) / (texelLength)) / CHUNK_SIZE - 1;
        }
    }

    // From buffer.chunkY to the chunk y is in
    int chunkDifferenceY(Position_t y)
    {
        if (y > buffer.y)
        {
            return -1 - ((int)((y - buffer.y) / (texelLength)) / CHUNK_SIZE);
        }
        else
        {
            return -1 * ((int)((y - buffer.y) / (texelLength)) / CHUNK_SIZE);
        }
    }

    bool chunkInRange()
    {

    }

    SDL_Texture* texture = nullptr;
    std::vector<std::vector<Chunk>> chunks;
    std::vector<std::vector<ChunkStatus_t>> chunksStatus;
    int width;  // In chunks
    int height; // In chunks
    
    Position_t texelLength;
    Buffer buffer;
    Range range;
};

Map::Map(int width, int height, Position_t texelLength):
    width(width), height(height), texelLength(texelLength)
{
    texture = SDL_CreateTexture(SDLManager::renderer(), SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width * CHUNK_SIZE, height * CHUNK_SIZE);

    RLOG("Texture size: " << width * CHUNK_SIZE << " * " << height * CHUNK_SIZE);

    chunks.resize(height);
    for (auto& i : chunks)
    {
        i.resize(width);
    }

    chunksStatus.resize(height);
    for (auto& i : chunksStatus)
    {
        i.resize(width);

        for (auto& j : i)
        {
            j = CHUNK_INIT;
        }
    }
}

Map::~Map()
{
    SDL_DestroyTexture(texture);
}

void Map::setRange(Position_t originX, Position_t originY, Position_t width, Position_t height)
{
    range = { originX, originY, width, height };

    RLOG("Range: (" << originX << ", " << originY << ") through (" << originX + width << ", " << originY - height << ")");
    RLOG("Old buffer Origin: (" << buffer.x << ", " << buffer.y << ")");
    RLOG("Old chunks: (" << buffer.chunkX << ", " << buffer.chunkY << ") through (" << buffer.chunkX + buffer.chunkWidth << ", " << buffer.chunkY + buffer.chunkHeight << ")");

    Buffer newBuffer;

    int chunkDX = chunkDifferenceX(originX);
    int chunkDY = chunkDifferenceY(originY);
    RLOG("Chunk Difference: (" << chunkDX << ", " << chunkDY << ")");

    newBuffer.x = buffer.x + chunkDX * texelLength * CHUNK_SIZE;
    newBuffer.y = buffer.y - chunkDY * texelLength * CHUNK_SIZE;

    // Modulo later
    newBuffer.chunkX = buffer.chunkX + chunkDX;
    newBuffer.chunkY = buffer.chunkY + chunkDY;

    newBuffer.chunkWidth = 1 + chunkDifferenceX(originX + width) - chunkDX;
    newBuffer.chunkHeight = 1 + chunkDifferenceY(originY - height) - chunkDY;
    RLOG("Chunk wh: (" << newBuffer.chunkWidth << ", " << newBuffer.chunkHeight << ")");

    for (int y = newBuffer.chunkY; y < newBuffer.chunkY + newBuffer.chunkHeight; ++y)
    {
        for (int x = newBuffer.chunkX; x < newBuffer.chunkX + newBuffer.chunkWidth; ++x)
        {
            //RLOG("Checking chunk: (" << x % this->width << ", " << y % this->height << ")");
            if (
                ((x >= buffer.x && x < buffer.x + buffer.chunkWidth) &&
                (y >= buffer.y && y < buffer.y + buffer.chunkHeight)) == false
            )
            {
                RLOG("Chunk: (" << (x + this->width) % this->width << ", " << (y + this->height) % this->height << ")");
                chunksStatus[(y + this->height) % this->height][(x + this->width) % this->width] |= CHUNK_SHOULD_COMPUTE_BIT;
            }
        }
    }

    buffer.x = newBuffer.x;
    buffer.y = newBuffer.y;
    buffer.chunkX = (newBuffer.chunkX + this->width) % this->width;
    buffer.chunkY = (newBuffer.chunkY + this->height) % this->height;
    buffer.chunkWidth = newBuffer.chunkWidth;
    buffer.chunkHeight = newBuffer.chunkHeight;

    RLOG("New buffer Origin: (" << buffer.x << ", " << buffer.y << ")");
    RLOG("New chunks: (" << buffer.chunkX << ", " << buffer.chunkY << ") through (" << buffer.chunkX + buffer.chunkWidth << ", " << buffer.chunkY + buffer.chunkHeight << ")");
}

void Map::update(Iteration_t threshold)
{
    for (int yNoMod = buffer.chunkY; yNoMod < buffer.chunkY + buffer.chunkHeight; ++yNoMod)
    {
        for (int xNoMod = buffer.chunkX; xNoMod < buffer.chunkX + buffer.chunkWidth; ++xNoMod)
        {
            int y = yNoMod % height;
            int x = xNoMod % width;
            ChunkStatus_t& status = chunksStatus[y][x];
            Chunk& chunk = chunks[y][x];

            if (status & CHUNK_SHOULD_COMPUTE_BIT)
            {
                RLOG("Chunk: (" << x << ", " << y << ")");

                status &= ~(CHUNK_SHOULD_COMPUTE_BIT | CHUNK_DONE_COMPUTE_BIT);

                //compute
                chunk.compute(
                    buffer.x + ((x + width - buffer.chunkX) % width) * CHUNK_SIZE * texelLength,
                    buffer.y - ((y + height - buffer.chunkY) % height) * CHUNK_SIZE * texelLength,
                    texelLength,
                    threshold
                );

                status = (CHUNK_DONE_COMPUTE_BIT | CHUNK_SHOULD_DRAW_BIT);
            }
            else if (status & CHUNK_SHOULD_DRAW_BIT)
            {
                status &= ~(CHUNK_SHOULD_DRAW_BIT | CHUNK_DONE_DRAW_BIT);

                //draw
                chunk.draw(texture, x, y, width);

                status |= CHUNK_DONE_DRAW_BIT;
            }
        }
    }
}

void Map::renderCopy(SDL_Texture* screen)
{
    Position_t x = borderX();
    Position_t y = borderY();
    RLOG("Border: (" << x << ", " << y << ")");

    SDL_SetRenderTarget(SDLManager::renderer(), screen);

    // Can go beyond border of texture
    SDL_Rect src = {
            (buffer.chunkX * CHUNK_SIZE + (int)((range.x - buffer.x) / texelLength)) % (width * CHUNK_SIZE),
            (buffer.chunkY * CHUNK_SIZE - (int)((range.y - buffer.y) / texelLength)) % (height * CHUNK_SIZE),
            range.width / texelLength,
            range.height / texelLength
    };
    RLOG(src.x << ", " << src.y << ", " << src.w << ", " << src.h);

    // Topleft of screen, bottomright on the texture
    SDL_Rect topleft = {
            src.x,
            src.y,
            width * CHUNK_SIZE - src.x,
            height * CHUNK_SIZE - src.y
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

#endif // !MAP_H
