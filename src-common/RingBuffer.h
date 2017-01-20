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
#include <iterator>

template <typename _valType, unsigned size>
struct RingBuffer_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = _valType;
    using pointer           = _valType*;
    using reference         = _valType&;
    using size_type         = unsigned;
    using difference_type   = int;
    
private:
    pointer         _buffer;
    size_type       _index;
    
    RingBuffer_iterator(pointer buf, std::size_t index)
    : _buffer(buf), _index(index)
    {
    }
    
    template<typename _vt, unsigned _sz>
    friend class RingBuffer;
    
    template<typename _valType1, typename _valType2, unsigned _sz>
    friend bool operator==(const RingBuffer_iterator<_valType1, _sz>& __x,
                           const RingBuffer_iterator<_valType2, _sz>& __y) noexcept;
    
    template<typename _valType1, typename _valType2, unsigned _sz>
    friend bool operator!=(const RingBuffer_iterator<_valType1, _sz>& __x,
                           const RingBuffer_iterator<_valType2, _sz>& __y) noexcept;
    
    template<typename _valType1, typename _valType2, unsigned _sz>
    friend bool operator<(const RingBuffer_iterator<_valType1, _sz>& __x,
                          const RingBuffer_iterator<_valType2, _sz>& __y) noexcept;
    
    template<typename _valType1, typename _valType2, unsigned _sz>
    friend bool operator>(const RingBuffer_iterator<_valType1, _sz>& __x,
                          const RingBuffer_iterator<_valType2, _sz>& __y) noexcept;
    
    template<typename _valType1, typename _valType2, unsigned _sz>
    friend bool operator<=(const RingBuffer_iterator<_valType1, _sz>& __x,
                           const RingBuffer_iterator<_valType2, _sz>& __y) noexcept;
    
    template<typename _valType1, typename _valType2, unsigned _sz>
    friend bool operator>=(const RingBuffer_iterator<_valType1, _sz>& __x,
                           const RingBuffer_iterator<_valType2, _sz>& __y) noexcept;
    
public:
    RingBuffer_iterator(const RingBuffer_iterator& rbi)
    : _buffer(rbi._buffer), _index(rbi._index)
    { }
    RingBuffer_iterator()
    : _buffer(nullptr), _index(0)
    { }
    
    RingBuffer_iterator& operator=(const RingBuffer_iterator& rhs)
    {
        _buffer = rhs._buffer;
        _index = rhs._index;
        return *this;
    }
    
    void swap(RingBuffer_iterator& o)
    {
        auto _ti  = _index;     _index  = o._index;     o._index  = _ti;
        auto _tbp = _buffer;    _buffer = o._buffer;    o._buffer = _tbp;
    }
    
    reference operator*() const
    { return *_get_current(); }
    
    pointer operator->() const
    { return _get_current(); }
    
    RingBuffer_iterator& operator++()
    {
        _index = (_index - 1) % size;
        return *this;
    }
    
    RingBuffer_iterator operator++(int)
    {
        RingBuffer_iterator tmp = *this;
        ++*this;
        return tmp;
    }
    
    RingBuffer_iterator& operator--()
    {
        _index = (_index + 1) % size;
        return *this;
    }
    
    RingBuffer_iterator operator--(int)
    {
        RingBuffer_iterator tmp = *this;
        --*this;
        return tmp;
    }
    
    RingBuffer_iterator& operator+=(difference_type n)
    {
        _index = (_index - n) % size;
        return *this;
    }
    
    RingBuffer_iterator operator+(difference_type n) const
    {
        RingBuffer_iterator tmp = *this;
        tmp += n;
        return tmp;
    }
    
    RingBuffer_iterator& operator-=(difference_type n)
    {
        _index = (_index + n) % size;
        return *this;
    }
    
    RingBuffer_iterator operator-(difference_type n) const
    {
        RingBuffer_iterator tmp = *this;
        tmp -= n;
        return tmp;
    }
    
    difference_type operator-(const RingBuffer_iterator& rbi) const
    { return rbi._index - _index; }
    
    reference operator[](difference_type n) const
    {
        return *_get_current(n);
    }
    
    bool operator<(const RingBuffer_iterator& rhs) const noexcept
    {
        return _index > rhs._index;
    }
    
    bool operator>(const RingBuffer_iterator& rhs) const noexcept
    {
        return _index < rhs._index;
    }
    
    bool operator<=(const RingBuffer_iterator& rhs) const noexcept
    {
        return _index >= rhs._index;
    }
    
    bool operator>=(const RingBuffer_iterator& rhs) const noexcept
    {
        return _index <= rhs._index;
    }
    
    bool operator==(const RingBuffer_iterator& rhs) const noexcept
    {
        return _index == rhs._index;
    }
    
    bool operator!=(const RingBuffer_iterator& rhs) const noexcept
    {
        return _index != rhs._index;
    }
    
private:
    pointer _get_current(difference_type n = 0) const
    {
        return _buffer + (_index - n) % size;
    }
};

// These allow comparison of const iterators with non-const iterators
template<typename _valType1, typename _valType2, unsigned size>
inline bool operator==(const RingBuffer_iterator<_valType1, size>& __x,
                       const RingBuffer_iterator<_valType2, size>& __y) noexcept
{
    static_assert(std::is_same<typename std::remove_const<_valType1>::type,
                               typename std::remove_const<_valType2>::type>::value,
                  "Types must match, modulo const");
    return __x._index == __y._index;
}

template<typename _valType1, typename _valType2, unsigned size>
inline bool operator!=(const RingBuffer_iterator<_valType1, size>& __x,
                       const RingBuffer_iterator<_valType2, size>& __y) noexcept
{
    static_assert(std::is_same<typename std::remove_const<_valType1>::type,
                               typename std::remove_const<_valType2>::type>::value,
                  "Types must match, modulo const");
    return __x._index != __y._index;
}

template<typename _valType1, typename _valType2, unsigned size>
inline bool operator<(const RingBuffer_iterator<_valType1, size>& __x,
                      const RingBuffer_iterator<_valType2, size>& __y) noexcept
{
    static_assert(std::is_same<typename std::remove_const<_valType1>::type,
                               typename std::remove_const<_valType2>::type>::value,
                  "Types must match, modulo const");
    return __x._index > __y._index;
}

template<typename _valType1, typename _valType2, unsigned size>
inline bool operator>(const RingBuffer_iterator<_valType1, size>& __x,
                      const RingBuffer_iterator<_valType2, size>& __y) noexcept
{
    static_assert(std::is_same<typename std::remove_const<_valType1>::type,
                               typename std::remove_const<_valType2>::type>::value,
                  "Types must match, modulo const");
    return __x._index > __y._index;
}

template<typename _valType1, typename _valType2, unsigned size>
inline bool operator<=(const RingBuffer_iterator<_valType1, size>& __x,
                       const RingBuffer_iterator<_valType2, size>& __y) noexcept
{
    static_assert(std::is_same<typename std::remove_const<_valType1>::type,
                               typename std::remove_const<_valType2>::type>::value,
              "Types must match, modulo const");
    return __x._index >= __y._index;
}

template<typename _valType1, typename _valType2, unsigned size>
inline bool operator>=(const RingBuffer_iterator<_valType1, size>& __x,
                       const RingBuffer_iterator<_valType2, size>& __y) noexcept
{
    static_assert(std::is_same<typename std::remove_const<_valType1>::type,
                               typename std::remove_const<_valType2>::type>::value,
                  "Types must match, modulo const");
    return __x._index <= __y._index;
}

template<typename _valType, unsigned size>
inline RingBuffer_iterator<_valType, size> operator+(std::ptrdiff_t __n,
                                                     const RingBuffer_iterator<_valType, size>& __x) noexcept
{ return __x.operator+(__n); }

template<typename _valType, unsigned size>
void swap(RingBuffer_iterator<_valType, size>& s,
          RingBuffer_iterator<_valType, size>& t) noexcept(noexcept(s.swap(t)))
{ s.swap(t); }
        

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
    using iterator               = RingBuffer_iterator<T, _size>;
    using const_iterator         = RingBuffer_iterator<const T, _size>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
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
    { return _buffer[(_pointer - (_ringSize - 1) + index) % _size]; }
    const T& back(unsigned index = 0) const
    { return _buffer[(_pointer - (_ringSize - 1) + index) % _size]; }
    
    T& operator[](unsigned index)
    { return _buffer[(_pointer - index) % _size]; }
    const T& operator[](unsigned index) const
    { return _buffer[(_pointer - index) % _size]; }
    
    T& at(unsigned index)
    {
        if (index >= _ringSize)
            throw std::out_of_range("RingBuffer::at index exceeds size");
        return _buffer[(_pointer - index) % _size];
    }
    const T& at(unsigned index) const
    {
        if (index >= _ringSize)
            throw std::out_of_range("RingBuffer::at index exceeds size");
        return _buffer[(_pointer - index) % _size];
    }
    
    unsigned size() const       { return _ringSize; }
    unsigned max_size() const   { return _size; }
    bool empty() const          { return _ringSize == 0; }
    
    RingBuffer(const RingBuffer&) = default;
    RingBuffer(RingBuffer&&) noexcept = default;
    RingBuffer& operator=(const RingBuffer&) = default;
    RingBuffer& operator=(RingBuffer&&) noexcept = default;
    
    iterator begin() noexcept             { return iterator(_buffer.data(), _pointer); }
    iterator end() noexcept               { return iterator(_buffer.data(), (_pointer - _ringSize) % _size); }
    const_iterator begin() const noexcept { return const_iterator(_buffer.data(), _pointer); }
    const_iterator end() const noexcept   { return const_iterator(_buffer.data(), (_pointer - _ringSize) % _size); }
    const_iterator cbegin() noexcept      { return const_iterator(_buffer.data(), _pointer); }
    const_iterator cend() noexcept        { return const_iterator(_buffer.data(), (_pointer - _ringSize) % _size); }
    
    reverse_iterator rbegin() noexcept             { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept               { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept   { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() noexcept      { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() noexcept        { return const_reverse_iterator(cbegin()); }

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
