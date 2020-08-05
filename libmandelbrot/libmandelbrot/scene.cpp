#include <limits>
#include "scene.h"

namespace mdb {

Scene::Scene(RectI drawArea, Number_t texelLength) :
    Maps{ Map(texelLength, U_SIZE, V_SIZE), Map(texelLength, U_SIZE, V_SIZE) },
    currentMap(Maps[0]), otherMap(Maps[1]),
    drawArea(drawArea)
{
    MDB_INFO("Texture size: {} * {}", U_SIZE * Chunk::SIZE, V_SIZE * Chunk::SIZE);

    static_assert
    (
        U_SIZE* V_SIZE* Chunk::SIZE* Chunk::SIZE <= std::numeric_limits<PixelDataIndex_t>::max(),
        "PixelDataIndex_t will overflow, texture is too large. Reduce Map U_SIZE/V_SIZE or Chunk::SIZE."
    );

    texture = Texture::Create(U_SIZE * Chunk::SIZE, V_SIZE * Chunk::SIZE, Texture::Access::STREAMING);
}

void Scene::Zoom(int zoomCenterX, int zoomCenterY, float multiplier)
{
    Number_t newPixelLength = pixelLength * multiplier;

    if (newPixelLength < currentMap.MinPixelLength())
    {
        MDB_TRACE("Zoom reached minimum");
        currentMap.ChangeTexelLength(currentMap.NextSmallerTexelLength(drawArea.w, drawArea.h));
    }
    else if (newPixelLength > currentMap.MaxPixelLength(drawArea.w, drawArea.h))
    {
        MDB_TRACE("Zoom reached maximum");
        currentMap.ChangeTexelLength(currentMap.PrevLargerTexelLength(drawArea.w, drawArea.h));
    }

    ZoomMovement(zoomCenterX, zoomCenterY, newPixelLength - pixelLength);
    SetNumberRange(range.x, range.y, newPixelLength);
}

void Scene::ZoomMovement(int x, int y, Number_t dPixelLength)
{
    range.x -= static_cast<Number_t>(x) * dPixelLength;
    range.y += static_cast<Number_t>(y) * dPixelLength;
}

void Scene::Movement(float x, float y)
{
    MDB_TRACE("Movement ({}, {}) {}", x, y, pixelLength);
    range.x += static_cast<Number_t>(x) * pixelLength;
    range.y -= static_cast<Number_t>(y) * pixelLength;
}

void Scene::SetNumberRange(Number_t originX, Number_t originY, Number_t pixelLength)
{           
    range = { originX, originY, drawArea.w * pixelLength, drawArea.h * pixelLength };
    this->pixelLength = pixelLength;
}

void Scene::Update(Iteration_t threshold)
{
    MDB_TRACE
    (
        "(x, y): ({}, {}) screen width: {} with pixellength {}",
        range.x, range.y, pixelLength * drawArea.w, pixelLength
    );
    currentMap.UpdateBuffer(range);
    currentMap.UpdateState(texture, threshold);
}

void Scene::Draw()
{
    currentMap.Draw(texture, range, pixelLength);
}

//void Scene::DebugDraw()
//{
//    texture->Draw(
//        { 0, 0, U_SIZE * Chunk::SIZE, V_SIZE * Chunk::SIZE },
//        { 0, 0, U_SIZE * 16, V_SIZE * 16 }
//    );
//}

void Scene::DebugDraw(RectF dst)
{
    texture->Draw(
        { 0, 0, U_SIZE * Chunk::SIZE, V_SIZE * Chunk::SIZE }, dst
    );
}

} // namespace mdb