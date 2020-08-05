#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <memory>
#include "common.h"

namespace mdb {

/***************************************************************
    Utility
***************************************************************/

template<typename T>
struct Rect
{
    T x, y, w, h;
};

typedef Rect<int> RectI;
typedef Rect<float> RectF;

/***************************************************************
    Interface
***************************************************************/

bool Initiallized();    // Call this to check if platform-specific setup is done

// Implement the following

void SetDrawAreaAsTarget();
void UnsetDrawAreaAsTarget();

class Texture
{
public:

    enum class Access {
        STATIC,
        STREAMING,
        TARGET
    };

    enum class Format {
        NATIVE
    };

    virtual void Draw(RectI srcRect, RectF dstRect) = 0;    // TODO: Change to rectF: rectI is only a sdl compromise
    virtual void SetAsTarget() = 0;
    virtual void UnsetAsTarget() = 0;

    virtual void Color(int u, int v, Iteration_t iteration, Iteration_t threshold) = 0;
    virtual void Update() = 0;

    [[nodiscard]] static std::unique_ptr<Texture> Create(
        int width, int height, Access access = Access::STATIC, Format format = Format::NATIVE
    );
};

} // namespace mdb

#endif // !GRAPHICS_H
