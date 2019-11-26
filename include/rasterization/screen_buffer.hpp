#pragma once

#include "rasterization/rendering_geometry.hpp"
#include <thread>
#include <atomic>
#include <limits>

namespace plane_render {

// Обеспечивает доступ 
class ScreenBuffer
{
public:
    static constexpr size_t LockFragment = 5; // Сколько рядов блокируются одновременно

public:
    // Обеспечивает спинлок линии
    class Accessor
    {
    public:
        static constexpr size_t INVALID_ROW = std::numeric_limits<size_t>::max();
    private:
        size_t row_ = INVALID_ROW; 
        ScreenBuffer* buffer_ = nullptr;

    public:
        Accessor(Accessor&& ac);
        Accessor& operator=(Accessor&& ac);
        ~Accessor();

        inline size_t LockedRow() const { return row_; }

        inline bool TryLockRow(size_t row)
        {
            ReleaseRow(); // Защита на всякий случай: но лучше заранее самому вызывать

            bool expected = false;
            if (buffer_->locks_[row].compare_exchange_weak(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            {
                row_ = row;
                return true;
            }
            return false;
        }

        inline void LockRow(size_t row)
        {
            while (!TryLockRow(row))
                std::this_thread::yield();
        }

        inline void ReleaseRow()
        {
            if (row_ != INVALID_ROW)
            {
                buffer_->locks_[row_].store(false, std::memory_order_release); // Мы владеем, поэтому просто store
                row_ = INVALID_ROW;
            }
        }

        inline Color& Pixel(size_t x)
        { 
            DCHECK(x < buffer_->width_ && row_ != INVALID_ROW);
            return buffer_->pixels_[buffer_->width_*row_ + x]; 
        }
        inline float& Z(size_t x) 
        {
            DCHECK(x < buffer_->width_ && row_ != INVALID_ROW);
            return buffer_->z_buffer_[buffer_->width_*row_ + x]; 
        }
    
    private:
        Accessor(ScreenBuffer* buff);
        Accessor(const Accessor&) = delete;
        Accessor& operator=(const Accessor&) = delete;

        friend class ScreenBuffer;
    };

public:
    ScreenBuffer(size_t w, size_t h);
    ScreenBuffer(const ScreenBuffer&) = delete;
    ScreenBuffer& operator=(const ScreenBuffer&) = delete;
    
    inline Accessor GetAccessor()
    {
        return Accessor(this);
    }

    // Функция без синхронизации - должна использоваться только ПОСЛЕ растеризации
    // В debug проверяет, что все спинлоки отпущены
    void Clear();
    
    const Color* GetPixels() const { return pixels_; }
    size_t GetBufferSize()   const { return width_*height_*sizeof(Color); }
    
    ~ScreenBuffer();

private:
    size_t width_;
    size_t height_;

    Color* pixels_ = nullptr;
    float* z_buffer_ = nullptr;

    std::vector<std::atomic_bool> locks_; // По 1 на ряд
};

} // namespace plane_render