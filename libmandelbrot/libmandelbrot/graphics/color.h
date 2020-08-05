#ifndef COLOR_H
#define COLOR_H

namespace mdb {

    struct Color
    {
        float r;
        float g;
        float b;

        constexpr static int size = 3;
    };

    constexpr int PALETTE_SIZE = 16;
    constexpr int INTERVAL = 16;
    constexpr int COLOR_COUNT = INTERVAL * PALETTE_SIZE;

    // Set draw color here
    constexpr Color BOUNDED_COLOR = { 0.0f, 0.0f, 0.0f };
    constexpr Color UNBOUNDED_COLORS[PALETTE_SIZE + 1] =
    {
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },

        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.5f, 0.0f },
        { 0.0f, 0.0f, 0.0f },

        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.5f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },

        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },

        { 0.0f, 0.0f, 1.0f }    // repeats the first color
    };

    [[nodiscard]] constexpr Color getInterpolated(int iterationsMod)
    {
        int b = iterationsMod / INTERVAL;
        int m = iterationsMod % INTERVAL;

        return
        {
            UNBOUNDED_COLORS[b].r + (UNBOUNDED_COLORS[b + 1].r - UNBOUNDED_COLORS[b].r) * ((float)m / (float)INTERVAL),
            UNBOUNDED_COLORS[b].g + (UNBOUNDED_COLORS[b + 1].g - UNBOUNDED_COLORS[b].g) * ((float)m / (float)INTERVAL),
            UNBOUNDED_COLORS[b].b + (UNBOUNDED_COLORS[b + 1].b - UNBOUNDED_COLORS[b].b) * ((float)m / (float)INTERVAL)
        };
    }

} // namespace mdb

#endif // !COLOR_H
