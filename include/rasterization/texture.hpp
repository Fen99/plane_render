#pragma once

#include "graphics_types.hpp"
#include "common/logger.hpp"

namespace plane_render {

class Texture
{
private:
    static constexpr size_t StorageBlockSide = 2; // Храним квадратиками, их сторона
    static constexpr size_t StorageBlockSize = StorageBlockSide * StorageBlockSide;

private:
    Color* texture_ = nullptr;

    size_t side_ = 0; // Сторона текстуры
    size_t blocks_by_side_ = 0; // Сколько блоков по каждой стороне

public:
    Texture() {}
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    const Color* GetPixels() const { return texture_; }

    void Load(const std::string& text_name);
    operator bool() const { return texture_ != nullptr; }

    inline const Color GetPoint(const TextureCoords& coords) const
    {
        DCHECK(texture_);
        DCHECK(coords.x < 1 && coords.y < 1 && coords.x >= 0 && coords.y >= 0);

        // Точка в большом квадрате. -1 - т.к. нумерация пикселей с 0 по каждой стороне
        Point2D<int> texture_point = { static_cast<int>(coords.x*(side_-1)), static_cast<int>((1-coords.y)*(side_-1)) };
        // Блок с данной точкой
        size_t block_id = blocks_by_side_*(texture_point.y / StorageBlockSide) + (texture_point.x / StorageBlockSide);
        size_t pos_in_block = StorageBlockSide*(texture_point.y % StorageBlockSide) + (texture_point.x % StorageBlockSide);

        const Color& result = texture_[block_id*StorageBlockSize + pos_in_block];
        return result;
    }

    ~Texture();
};

} // namespace plane_render