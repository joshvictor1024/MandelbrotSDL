#include <limits>
#include "scene.h"

Scene::Scene(SDL_Rect drawArea, Number_t texelLength):
    Maps{ Map(texelLength, U_SIZE, V_SIZE), Map(texelLength, U_SIZE, V_SIZE) }, currentMap(Maps[0]), otherMap(Maps[1]), drawArea(drawArea)
{
    ILOG("Texture size: " << U_SIZE * Chunk::SIZE << " * " << V_SIZE * Chunk::SIZE);
    DLOG("Texture texel count: " << U_SIZE * V_SIZE * Chunk::SIZE * Chunk::SIZE);
    DLOG("PixelDataIndex_t max value: " << std::numeric_limits<PixelDataIndex_t>::max());

    static_assert
    (
        U_SIZE* V_SIZE* Chunk::SIZE* Chunk::SIZE <= std::numeric_limits<PixelDataIndex_t>::max(),
        "PixelDataIndex_t will overflow, texture is too large. Reduce Map U_SIZE/V_SIZE or Chunk::SIZE."
    );

    texture = SDL_CreateTexture
    (
        SDLManager::renderer(), SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
        U_SIZE * Chunk::SIZE, V_SIZE * Chunk::SIZE
    );
}

Scene::~Scene()
{
    SDL_DestroyTexture(texture);
}

void Scene::Zoom(int x, int y, float multiplier)
{
    Number_t newPixelLength = pixelLength * multiplier;

    if (newPixelLength < currentMap.MinPixelLength())
    {
        DLOG("min\n");
        //DLOG("texelLength: " << currentMap.MinPixelLength());
        //DLOG("texelLength: " << Map::TexelLengthFromMax(currentMap.MaxPixelLength(drawArea.w, drawArea.h), drawArea.w, drawArea.h));

        currentMap = Map(currentMap.NextSmallerTexelLength(drawArea.w, drawArea.h), std::move(currentMap));
    }
    else if (newPixelLength > currentMap.MaxPixelLength(drawArea.w, drawArea.h))
    {
        DLOG("max\n");
        //DLOG("texelLength: " << currentMap.MinPixelLength());
        //DLOG("texelLength: " << Map::TexelLengthFromMin(currentMap.MaxPixelLength(drawArea.w, drawArea.h)));

        currentMap = Map(currentMap.PrevLargerTexelLength(drawArea.w, drawArea.h), std::move(currentMap));
    }

    ZoomMovement(x, y, newPixelLength - pixelLength);
    SetNumberRange(range.x, range.y, newPixelLength);
}

//void Scene::Zoom(int x, int y, float multiplier, int deltaTime/*in miliseconds*/)
//{
//    RLOG("multiplier: " << multiplier << " deltaTime: " << deltaTime << "new multiplier: " << multiplier * float(deltaTime) * 0.001);
//    Zoom(x, y, multiplier * float(deltaTime) * 0.001);
//}

void Scene::ZoomIn(int x, int y)
{
    Number_t newPixelLength = pixelLength * (1.0 / SCROLL_ZOOM_OUT);

    if (newPixelLength < currentMap.MinPixelLength())
    {
        DLOG("min\n");
        //DLOG("texelLength: " << currentMap.MinPixelLength());
        //DLOG("texelLength: " << Map::TexelLengthFromMax(currentMap.MaxPixelLength(drawArea.w, drawArea.h), drawArea.w, drawArea.h));

        currentMap = Map(currentMap.NextSmallerTexelLength(drawArea.w, drawArea.h), std::move(currentMap));
    }

    ZoomMovement(x, y, newPixelLength - pixelLength);
    SetNumberRange(range.x, range.y, newPixelLength);
}

void Scene::ZoomOut(int x, int y)
{
    Number_t newPixelLength = pixelLength * SCROLL_ZOOM_OUT;

    if (newPixelLength > currentMap.MaxPixelLength(drawArea.w, drawArea.h))
    {
        DLOG("max\n");
        //DLOG("texelLength: " << currentMap.MinPixelLength());
        //DLOG("texelLength: " << Map::TexelLengthFromMin(currentMap.MaxPixelLength(drawArea.w, drawArea.h)));

        currentMap = Map(currentMap.PrevLargerTexelLength(drawArea.w, drawArea.h), std::move(currentMap));
    }

    ZoomMovement(x, y, newPixelLength - pixelLength);
    SetNumberRange(range.x, range.y, newPixelLength);
}

void Scene::ZoomMovement(int x, int y, Number_t dPixelLength)
{
    range.x -= static_cast<Number_t>(x) * dPixelLength;
    range.y += static_cast<Number_t>(y) * dPixelLength;
}

void Scene::Movement(int x, int y)
{
    DLOG("Movement " << x << " " << y << " " << pixelLength);
    range.x += static_cast<Number_t>(x) * pixelLength;
    range.y -= static_cast<Number_t>(y) * pixelLength;
}

void Scene::Movement(int x, int y, int deltaTime/*in miliseconds*/)
{
    DLOG("Movement " << x << " " << y << " " << pixelLength);
    range.x += static_cast<Number_t>(x * deltaTime) * pixelLength * 0.001;
    range.y -= static_cast<Number_t>(y * deltaTime) * pixelLength * 0.001;
}

void Scene::SetNumberRange(Number_t originX, Number_t originY, Number_t pixelLength)
{           
    range = { originX, originY, drawArea.w * pixelLength, drawArea.h * pixelLength };
    this->pixelLength = pixelLength;
}

void Scene::Update(Iteration_t threshold)
{
    ILOG
    (
        "(x, y): (" <<
        range.x << ", " <<
        range.y << ") screen width: " <<
        pixelLength * drawArea.w << " (pixelLength: " <<
        pixelLength << " )"
    );
    currentMap.UpdateBuffer(range);
    currentMap.UpdateState(texture, threshold);
}

void Scene::RenderCopy(SDL_Texture* screen)
{
    currentMap.RenderCopy(screen, texture, range, pixelLength);
}