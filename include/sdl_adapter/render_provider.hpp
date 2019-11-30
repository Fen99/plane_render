#pragma once

#include "common/common_graphics.hpp"
#include <memory>

namespace plane_render {

// Общий интерфейс классов, обеспечивающих рендер и сцены и управление камерой
class IRenderProvider
{
public:
    virtual void Update() = 0; // Перерисовывает экран

    // Управление камерой и видом
    virtual void MoveCam(float dx, float dy, float dz) = 0;
    virtual void MoveAt (float dx, float dy, float dz) = 0;

    virtual const Color* GetPixels() const = 0;
    virtual size_t GetBufferSize()   const = 0;
    virtual ScreenDimension ScreenWidth()  const = 0;
    virtual ScreenDimension ScreenHeight() const = 0;

};

typedef std::shared_ptr<IRenderProvider> RenderProviderPtr;

} // namespace plane_render