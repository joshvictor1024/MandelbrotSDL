#ifndef SCENE_H
#define SCENE_H

#include <array>
#include "common.h"
#include "graphics.h"
#include "map.h"

namespace mdb {

// Interface for controlling number range
// Controls Map switching
class Scene
{
public:

    Scene(RectI drawArea, Number_t texelLength);

    // TODO: Support non-zero origin
    // TODO: support varying drawArea e.g. varying window size

    void Zoom(int zoomCenterX, int zoomCenterY, float multiplier);

    // In pixels
    void Movement(float x, float y);

    void SetNumberRange(Number_t originX, Number_t originY, Number_t pixelLength);
    void Update(Iteration_t threshold);
    void Draw();
    //void DebugDraw();
    void DebugDraw(RectF dst);

    void Recompute()
    {
        currentMap.Recompute();
    }

private:

    void ZoomMovement(int x, int y, Number_t dPixelLength);

    constexpr static Chunk_t U_SIZE = 20;
    constexpr static Chunk_t V_SIZE = 13;

    std::unique_ptr<Texture> texture;
    
    std::array<Map, 2> Maps;
    Map& currentMap;
    Map& otherMap;

    NumberRange range;
    Number_t pixelLength;   // TODO: default pixelLength?
    RectI drawArea;
};

} // namespace mdb

#endif // !SCENE_H
