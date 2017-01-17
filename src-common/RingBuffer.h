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
#include <memory>

template <class T>
class RingBuffer
{
    std::unique_ptr<T[]> buffer;
    unsigned bufferSize, ringSize;
    unsigned pointer;
public:
    RingBuffer(): bufferSize(0), ringSize(0), pointer(0)    {}
    RingBuffer(unsigned sz, T init): bufferSize(0), ringSize(0), pointer(0)
    { resetBuffer(sz, init); }
    ~RingBuffer() = default;
    void resetBuffer(unsigned sz, T init);
    void setSize(unsigned sz);
    void add(T item);
    T tail(unsigned index) const
    { return buffer[(pointer + bufferSize - ringSize + index + 1) % bufferSize]; }
    T get(unsigned index) const
    { return buffer[(pointer + bufferSize - index) % bufferSize]; }
    unsigned length() const      { return ringSize; }

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
};

template <class T>
void RingBuffer<T>::resetBuffer(unsigned sz, T init)
{
    buffer = std::make_unique<T[]>(sz);
    bufferSize = ringSize = sz;
    for (unsigned i = 0; i < bufferSize; i++) buffer[i] = init;
    pointer = 0;
}

template <class T>
void RingBuffer<T>::add(T item)
{
    pointer = (pointer + 1) % bufferSize;
    buffer[pointer] = item;
}

template <class T>
void RingBuffer<T>::setSize(unsigned sz)
{
#if DEBUG
    assert(sz <= bufferSize);
#endif
    ringSize = sz; 
}

#endif // INCLUDED_RINGBUFFER
