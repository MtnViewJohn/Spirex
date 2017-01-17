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

template <class T, unsigned size>
class RingBuffer
{
    std::array<T, size> buffer;
    unsigned ringSize;
    unsigned pointer;
public:
    RingBuffer(): ringSize(0), pointer(0)    {}
    RingBuffer(T init): ringSize(0), pointer(0)
    { resetBuffer(init); }
    ~RingBuffer() = default;
    void resetBuffer(unsigned sz, T init);
    void setSize(unsigned sz);
    void add(T item);
    T tail(unsigned index) const
    { return buffer[(pointer + size - ringSize + index + 1) % size]; }
    T get(unsigned index) const
    { return buffer[(pointer + size - index) % size]; }
    unsigned length() const      { return ringSize; }

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;
};

template <class T, unsigned size>
void RingBuffer<T, size>::resetBuffer(unsigned sz, T init)
{
    ringSize = sz;
    pointer = 0;
    for (T& e: buffer)
        e = init;
}

template <class T, unsigned size>
void RingBuffer<T, size>::add(T item)
{
    pointer = (pointer + 1) % size;
    buffer[pointer] = item;
}

template <class T, unsigned size>
void RingBuffer<T, size>::setSize(unsigned sz)
{
#if DEBUG
    assert(sz <= size);
#endif
    ringSize = sz; 
}

#endif // INCLUDED_RINGBUFFER
