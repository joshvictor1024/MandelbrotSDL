#include <complex>
#include "chunk.h"

namespace mdb {

typedef std::complex<Number_t> Complex_t;//TODO: not necessary?

constexpr Number_t absSquared(const Complex_t& c) noexcept
{
    return (c.real() * c.real() + c.imag() * c.imag());
}

void Chunk::Compute(Number_t originX, Number_t originY, Number_t texelLength, Iteration_t threshold)
{
    int index = 0;

    for (int texelV = 0; texelV < SIZE; ++texelV)
    {
        for (int texelU = 0; texelU < SIZE; ++texelU, ++index)
        {
            Complex_t c = { 0.0, 0.0 };
            Complex_t dc =
            {
                originX + texelU * texelLength,
                originY - texelV * texelLength
            };

            Iteration_t it = 0;
            for (; it < threshold; ++it)
            {
                c = c * c + dc;

                if (absSquared(c) > static_cast<Number_t>(2 * 2))
                {
                    break;
                }
            }

            iterations[index] = it;
        }
    }
}

void Chunk::Draw(std::unique_ptr<Texture>& texture, Chunk_t chunkUMod, Chunk_t chunkVMod, Iteration_t threshold)
{
    for (int v = 0; v < SIZE; ++v)
    {
        for (int u = 0; u < SIZE; ++u)
        {
            texture->Color(u + (chunkUMod * SIZE), v + (chunkVMod * SIZE), iterations[u + v * SIZE], threshold);
        }
    }
}

} // namespace mdb