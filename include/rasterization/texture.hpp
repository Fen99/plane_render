#pragma once

#include "graphics_types.hpp"

#include "easyppm.h"

namespace plane_render {

class Texture
{
public:
    Texture(const std::string& text_name);
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    const Color& GetPoint(const TextureCoords& coords) const;

    ~Texture();

private:
    static constexpr size_t StorageBlock = 16; // Храним квадратиками

public:
    Color* texture_ = nullptr;
    size_t size_ = 0; // Сторона текстуры
};

} // namespace plane_render