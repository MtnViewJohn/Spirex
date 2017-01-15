// Spirex Screensaver
// ---------------------
// Copyright (C) 2001, 2002, 2003 John Horigan
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

#include <assert.h>
#include <memory>

template <class T>
class RingBuffer 
{
	std::unique_ptr<T[]> buffer;
	int bufferSize, ringSize;
	int pointer;
public:
	RingBuffer():buffer(0),bufferSize(0),ringSize(0),pointer(0)	{}
	RingBuffer(int sz, T init):buffer(0),bufferSize(0),ringSize(0),pointer(0)	
		{resetBuffer(sz, init);}
    ~RingBuffer() = default;
	void resetBuffer(int sz, T init);
	void setSize(int sz);
	void add(T item);
	T tail(int index) const 
		{ return buffer[(pointer + bufferSize - ringSize + index + 1) % bufferSize]; }
	T get(int index) const	
		{ return buffer[(pointer + bufferSize - index) % bufferSize]; }
	int length() const		{ return ringSize; }
private:
	RingBuffer(const RingBuffer&);
	RingBuffer& operator=(const RingBuffer&);
		// not implemented, can't be copied or assigned
};

template <class T>
void RingBuffer<T>::resetBuffer(int sz, T init)
{
    buffer = std::make_unique<T[]>(sz);
	bufferSize = ringSize = sz;
	for (int i = 0; i < bufferSize; i++) buffer[i] = init;
	pointer = 0;
}

template <class T>
void RingBuffer<T>::add(T item)
{
	pointer = (pointer + 1) % bufferSize;
	buffer[pointer] = item;
}

template <class T>
void RingBuffer<T>::setSize(int sz)	
{ 
#if DEBUG
	assert(sz <= bufferSize); 
#endif	
	ringSize = sz; 
}

#endif // INCLUDED_RINGBUFFER