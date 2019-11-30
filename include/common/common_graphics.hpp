#pragma once

// Общие типы для графики - чтобы обеспечить совместимость на уровне SDLAdapter
namespace plane_render {

template<typename T>
struct Point2D
{
    T x;
    T y;
};

// Screen coordinates (in pixels)
typedef int ScreenDimension;
using PixelPoint = Point2D<ScreenDimension>;
using PixelPointF = Point2D<float>; // Для ускорения некоторых расчетов, храним все равно целые

// Colors
struct Color // Order for SDL
{
public:
    typedef unsigned char ColorElement;

public:
    ColorElement A = 0;
    ColorElement R = 0;
    ColorElement G = 0;
    ColorElement B = 0;

public:
    inline Color operator*(float val) const
    {
        return { static_cast<ColorElement>(A*val),
                 static_cast<ColorElement>(R*val),
                 static_cast<ColorElement>(G*val),
                 static_cast<ColorElement>(B*val) };
    }
};
constexpr Color Red   = { 0, 255, 0, 0 };
constexpr Color Green = { 0, 0, 255, 0 };
constexpr Color Blue  = { 0, 0, 0, 255 };

} // namespace plane_render