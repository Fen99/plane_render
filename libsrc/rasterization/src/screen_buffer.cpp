#include "screen_buffer.hpp"

namespace plane_render {

ScreenBuffer::Accessor::Accessor(Accessor&& ac) : row_(ac.row_), buffer_(ac.buffer_)
{}

ScreenBuffer::Accessor& ScreenBuffer::Accessor::operator=(Accessor&& ac)
{
    ReleaseRow();
    row_ = ac.row_;
    buffer_ = ac.buffer_;
    return *this;
}

ScreenBuffer::Accessor::~Accessor()
{
    ReleaseRow();
}

ScreenBuffer::Accessor::Accessor(ScreenBuffer* buff) : buffer_(buff)
{}

ScreenBuffer::ScreenBuffer(size_t w, size_t h) : width_(w), height_(h), locks_(height_)
{
    pixels_ = new Color[w*h];
    z_buffer_ = new float[w*h];
    CHECK(pixels_ && z_buffer_);

    for (size_t i = 0; i < height_; i++)
        locks_[i].store(false);
}

ScreenBuffer::~ScreenBuffer()
{
    delete[] pixels_;
    delete[] z_buffer_;
}

void ScreenBuffer::Clear()
{
    for (size_t row = 0; row < height_; row++)
        DCHECK(!locks_[row].load());

    for (size_t i = 0; i < width_*height_; i++)
        z_buffer_[i] = -std::numeric_limits<float>::max();
    memset(pixels_, 0, width_*height_*sizeof(Color));
}

} // namespace plane_render