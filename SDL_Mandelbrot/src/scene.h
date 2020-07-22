#ifndef SCENE_H
#define SCENE_H

#include <array>
#include "constants.h"
#include "sdl_manager.h"
#include "map.h"

class Scene
{
public:

    Scene(SDL_Rect drawArea, Number_t texelLength);
    ~Scene();

    // TODO: Support non-zero origin
    // TODO: support varying drawArea e.g. varying window size

    void Zoom(int x, int y, float multiplier);
    //void Zoom(int x, int y, float multiplier, int deltaTime/*in miliseconds*/);
    void ZoomIn(int x, int y);      // Magnifiy, see a smaller area
    void ZoomOut(int x, int y);     // See a larger area
    void Movement(int x, int y);
    void Movement(int x, int y, int deltaTime/*in miliseconds*/);

    void SetNumberRange(Number_t originX, Number_t originY, Number_t pixelLength);
    void Update(Iteration_t threshold);
    void RenderCopy(SDL_Texture* screen);
    void DebugCopy()
    {
        SDL_Rect dst = { 0, 0, U_SIZE * 16, V_SIZE * 16 };
        SDL_RenderCopy(SDLManager::renderer(), texture, nullptr, &dst);
    }

private:

    void ZoomMovement(int x, int y, Number_t dPixelLength);

    SDL_Texture* texture = nullptr;

    constexpr static Chunk_t U_SIZE = 20;
    constexpr static Chunk_t V_SIZE = 13;
    
    std::array<Map, 2> Maps;
    Map& currentMap;
    Map& otherMap;

    NumberRange range;
    Number_t pixelLength;   // TODO: default pixelLength?
    SDL_Rect drawArea;
};

#endif // !SCENE_H
