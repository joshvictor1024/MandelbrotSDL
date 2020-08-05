#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

namespace mdb {

typedef double          Number_t;
typedef std::uint16_t   Iteration_t;
typedef std::int16_t    Chunk_t;        // Could technically use int8_t, but char does't play nice with text output
typedef std::uint32_t   PixelDataIndex_t;

} // namespace mdb

#include "log.h"

#endif // !CONSTANTS_H
