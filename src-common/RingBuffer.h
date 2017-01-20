// Spirex Screensaver
// ---------------------
// Copyright (C) 2001-2017 John Horigan
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// John Horigan can be contacted at john@glyphic.com or at
// John Horigan, 1209 Villa St., Mountain View, CA 94041-1123, USA
//
//

#ifndef INCLUDED_RINGBUFFER
#define INCLUDED_RINGBUFFER

#include <cassert>
#include <array>


template <class T, unsigned _size>
class RingBuffer
{
    std::array<T, _size> _buffer;
    unsigned _ringSize;
    unsigned _pointer;
    
public:
    using value_type             = T;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using size_type              = unsigned;
    using difference_type        = int;

    RingBuffer(): _ringSize(0), _pointer(0)    {}
    RingBuffer(unsigned sz, T init): _ringSize(0), _pointer(0)
    { resetBuffer(sz, init); }
    ~RingBuffer() = default;
    void resetBuffer(unsigned sz, const T& init);
    void setSize(unsigned sz);
    void add(const T& item);
    
    T& front() noexcept
    { return _buffer[_pointer]; }
    const T& front() const
    { return _buffer[_pointer]; }
    
    T& back(unsigned index = 0)
    { return _buffer[(_pointer + _size - (_ringSize - 1) + index) % _size]; }
    const T& back(unsigned index = 0) const
    { return _buffer[(_pointer + _size - (_ringSize - 1) + index) % _size]; }
    
    T& operator[](unsigned index)
    { return _buffer[(_pointer + _size - index) % _size]; }
    const T& operator[](unsigned index) const
    { return _buffer[(_pointer + _size - index) % _size]; }
    
    T& at(unsigned index)
    {
        if (index >= _ringSize)
            throw std::out_of_range("RingBuffer::at index exceeds size");
        return _buffer[(_pointer + _size - index) % _size];
    }
    const T& at(unsigned index) const
    {
        if (index >= _ringSize)
            throw std::out_of_range("RingBuffer::at index exceeds size");
        return _buffer[(_pointer + _size - index) % _size];
    }
    
    unsigned size() const       { return _ringSize; }
    unsigned max_size() const   { return _size; }
    bool empty() const          { return _ringSize == 0; }
    
    RingBuffer(const RingBuffer&) = default;
    RingBuffer(RingBuffer&&) noexcept = default;
    RingBuffer& operator=(const RingBuffer&) = default;
    RingBuffer& operator=(RingBuffer&&) noexcept = default;
    
    void swap(RingBuffer<T, _size>& other) noexcept(noexcept(_buffer.swap(other._buffer)))
    {
        _buffer.swap(other._buffer);
        unsigned t1 = _ringSize; _ringSize = other._ringsize; other._ringsize = t1;
        unsigned t2 = _pointer;   _pointer = other._pointer;   other._pointer = t2;
    }
};

template <class T, unsigned _size>
void RingBuffer<T, _size>::resetBuffer(unsigned sz, const T& init)
{
    if (sz > _size)
        throw std::out_of_range("RingBuffer::resetBuffer size exceeds capacity");
    _ringSize = sz;
    _pointer = 0;
    for (T& e: _buffer)
        e = init;
}

template <class T, unsigned _size>
void RingBuffer<T, _size>::add(const T& item)
{
    _pointer = (_pointer + 1) % _size;
    _buffer[_pointer] = item;
}

template <class T, unsigned _size>
void RingBuffer<T, _size>::setSize(unsigned sz)
{
    if (sz > _size)
        throw std::out_of_range("RingBuffer::setSize size exceeds capacity");
    _ringSize = sz;
}

#endif // INCLUDED_RINGBUFFER
