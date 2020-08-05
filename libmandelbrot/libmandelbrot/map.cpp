#include <future>
#include <thread>
#include <chrono>
#include "map.h"
#include "log.h"

namespace mdb {

/***************************************************************
    Utility
***************************************************************/

// TODO: Make these work for all integral/floating-point types

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

/***************************************************************
    Class
***************************************************************/

void BufferChunks::debugPrint()
{
    MDB_TRACE("Number (x, y): ({}, {})", x, y);
    MDB_TRACE(
        "Chunks: from ({}, {}) inclusive, to ({}, {}) exclusive",
        u, v, u + uSize, v + vSize
    );
}

std::atomic<int> Map::nowComputing{ 0 };

Map::~Map()
{
    MDB_INFO("Waiting for computations to end...");

    using namespace std::chrono_literals;
    constexpr auto WAIT_TIME = 10ms;

    while (nowComputing.load() != 0)
    {
        std::this_thread::sleep_for(WAIT_TIME);
    }
}

void Map::UpdateBuffer(NumberRange range)
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
        chunkDu > uSize || chunkDu < -1 * uSize ||
        chunkDv > vSize || chunkDv < -1 * vSize
        )
    {
        MDB_TRACE("All chunks in bound need to be computed!");

        // Fitting new u, v to old buffer

        Chunk_t chunkDuMod = floorModulo(chunkDu, static_cast<Number_t>(uSize));
        Chunk_t chunkDvMod = floorModulo(chunkDv, static_cast<Number_t>(vSize));

        buffer.u = floorModulo(static_cast<Chunk_t>(buffer.u + chunkDuMod), uSize);
        buffer.v = floorModulo(static_cast<Chunk_t>(buffer.v + chunkDvMod), vSize);

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
                Chunk_t uMod = u % uSize;
                Chunk_t vMod = v % vSize;

                MDB_TRACE(
                    "Mark to be re-computed, chunk: ({}, {}) i.e. ({}, {}) floorModulo",
                    u, v, uMod, vMod
                );

                chunksStatus[vMod][uMod] |= Chunk::SHOULD_COMPUTE_BIT;
            }
        }
    }
    else    // TODO: better old chunk recognition with chunkcoord
    {
        MDB_TRACE("Not all chunks in bound need to be computed.");

        // Modulo later when copying data back to buffer
        // Non-modulo value needed to check bounds

        newBuffer.u = buffer.u + chunkDu;
        newBuffer.v = buffer.v + chunkDv;

        // Check chunks in new bound (newBuffer), if they need to be re-computed

        for (Chunk_t v = newBuffer.v; v < newBuffer.v + newBuffer.vSize; ++v)
        {
            for (Chunk_t u = newBuffer.u; u < newBuffer.u + newBuffer.uSize; ++u)
            {
                Chunk_t uMod = floorModulo(u, uSize);
                Chunk_t vMod = floorModulo(v, vSize);

                // If the chunks are out of the original bound
                //ILOG("Checking chunk: (" << u << ", " << v << ")");
                if (
                    (u >= buffer.u && u < buffer.u + buffer.uSize &&
                        v >= buffer.v && v < buffer.v + buffer.vSize) == false
                    )
                {
                    MDB_TRACE(
                        "Mark to be re-computed, chunk: ({}, {}) i.e. ({}, {}) floorModulo",
                        u, v, uMod, vMod
                    );

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
    }

    buffer.debugPrint();
}

static std::vector<std::future<void>> futures;

void Map::UpdateState(std::unique_ptr<Texture>& texture, Iteration_t threshold)
{
    bool hasDrawn = false;

    for (Chunk_t v = buffer.v; v < buffer.v + buffer.vSize; ++v)
    {
        for (Chunk_t u = buffer.u; u < buffer.u + buffer.uSize; ++u)
        {
            // floorModulo not needed: stays positive and within +1 modulo
            Chunk_t uMod = u % uSize;
            Chunk_t vMod = v % vSize;
            Chunk::Status_t& status = chunksStatus[vMod][uMod];
            Chunk& chunk = chunks[vMod][uMod];

            if (status & Chunk::SHOULD_COMPUTE_BIT)
            {
                //ILOG("Chunk: (" << uMod << ", " << vMod << ")");

                // Atomic, but there are no synchronization or ordering constraints
                // ++nowComputing
                nowComputing.fetch_add(1, std::memory_order_relaxed);

                status &= ~Chunk::SHOULD_COMPUTE_BIT;

                auto compute = [&status, &chunk, this, u, v] (Iteration_t threshold)
                {
                    chunk.Compute
                    (
                        buffer.x + (u - buffer.u) * chunkLength,
                        buffer.y - (v - buffer.v) * chunkLength,
                        texelLength,
                        threshold
                    );

                    status |= Chunk::SHOULD_DRAW_BIT;

                    // Atomic, but there are no synchronization or ordering constraints
                    // --nowComputing
                    nowComputing.fetch_sub(1, std::memory_order_relaxed);
                };

                futures.push_back
                (
                    std::async(std::launch::async, compute, threshold)
                );
            }
            else if (status & Chunk::SHOULD_DRAW_BIT)
            {
                status &= ~Chunk::SHOULD_DRAW_BIT;

                if (hasDrawn == false)
                {
                    hasDrawn = true;
                }

                chunk.Draw(texture, uMod, vMod, threshold);
            }
        }
    }

    if (hasDrawn)
    {
        texture->Update();
    }

    if (nowComputing.load() == 0)
    {
        futures.clear();
    }
}

void Map::Draw(std::unique_ptr<Texture>& source, NumberRange range, Number_t pixelLength)
{
    Number_t x = Right();
    Number_t y = Bottom();
    MDB_TRACE("Border: ({}, {})", x, y);

    Number_t texelPerPixel = texelLength / pixelLength;

    // Can go beyond border of texture
    // floorModulo not needed: stays positive and within +1 modulo
    RectI src = {
            (buffer.u * Chunk::SIZE + (int)((range.x - buffer.x) / texelLength)) % (uSize * Chunk::SIZE),
            (buffer.v * Chunk::SIZE - (int)((range.y - buffer.y) / texelLength)) % (vSize * Chunk::SIZE),
            range.width / pixelLength,
            range.height / pixelLength
    };
    MDB_TRACE("src (x, y, w, h): ({}, {}, {}, {})", src.x, src.y, src.w, src.h);

    SetDrawAreaAsTarget();

    // Topleft of screen, bottomright on the texture
    RectI topleft = {
            src.x,
            src.y,
            uSize * Chunk::SIZE - src.x,
            vSize * Chunk::SIZE - src.y
    };
    MDB_TRACE("topleft (x, y, w, h): ({}, {}, {}, {})", topleft.x, topleft.y, topleft.w, topleft.h);
    {
        RectF dst = { 0, 0, topleft.w * texelPerPixel, topleft.h * texelPerPixel };
        MDB_TRACE("dst (x, y, w, h): ({}, {}, {}, {})", dst.x, dst.y, dst.w, dst.h);

        source->Draw(topleft, dst);
    }

    // Topright of screen, bottomleft on the texture
    MDB_TRACE("if topleft.w ({}) < range.width * texelLength ({})", topleft.w, range.width / texelLength);
    if (topleft.w < range.width / texelLength)
    {
        RectI topright = {
            0,
            src.y,
            range.width / texelLength - topleft.w,
            topleft.h
        };
        RectF dst = { topleft.w * texelPerPixel, 0, topright.w * texelPerPixel, topright.h * texelPerPixel };
        MDB_TRACE("dst (x, y, w, h): ({}, {}, {}, {})", dst.x, dst.y, dst.w, dst.h);

        source->Draw(topright, dst);
    }

    // Bottomleft of screen, topright on the texture
    MDB_TRACE("if topleft.h ({}) < range.height * texelLength ({})", topleft.h, range.height / texelLength);
    if (topleft.h < range.height / texelLength)
    {
        {
            RectI bottomleft = {
                src.x,
                0,
                topleft.w,
                range.height / texelLength - topleft.h
            };
            RectF dst = { 0, topleft.h * texelPerPixel, bottomleft.w * texelPerPixel, bottomleft.h * texelPerPixel };
            MDB_TRACE("dst (x, y, w, h): ({}, {}, {}, {})", dst.x, dst.y, dst.w, dst.h);

            source->Draw(bottomleft, dst);
        }

        // Bottomright of screen, topleft on the texture
        {
            RectI bottomright = {
                0,
                0,
                range.width / texelLength - topleft.w,
                range.height / texelLength - topleft.h
            };
            RectF dst = { topleft.w * texelPerPixel, topleft.h * texelPerPixel, bottomright.w * texelPerPixel, bottomright.h * texelPerPixel };
            MDB_TRACE("dst (x, y, w, h): ({}, {}, {}, {})", dst.x, dst.y, dst.w, dst.h);

            source->Draw(bottomright, dst);
        }
    }

    UnsetDrawAreaAsTarget();
}

} // namespace mdb