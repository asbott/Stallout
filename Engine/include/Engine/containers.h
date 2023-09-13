#pragma once

#include "Engine/memory.h"



/* 
    !! NOT RECOMMENDED FOR OOP.
    !! DOES NOT ENSURE OBJECT INTEGRITY.

    An Array (vector) with same functionality but skipping some
    C++ standards in exchange for speed, but using it requires
    100% understanding of what that means or extremely annoying
    UB bugs will appear.

    This array does not call ctor, dtor, copy dtor etc on resizing
    the buffer. Construction only happens when an item is added and
    destruction happens when it's removed (or array destructs).

    Other than that it should follow a regular STL vector implementation.

    Could also be used to linearly store non-copyable items but BEWARE
*/
template <typename type_t>
struct Forbidden_Array {

#ifndef ALLOW_NON_TRIVIAL_TYPES_FORBIDDEN_ARRAY
    static_assert(std::is_trivially_copyable<type_t>::value, "Forbidden_Array is only recommended for trivially copyable type. To bypass this define ALLOW_NON_TRIVIAL_TYPES_FORBIDDEN_ARRAY");
#endif

    static constexpr size_t type_size = sizeof(type_t);

    byte_t* _buffer_head = NULL;
    size_t _buffer_size = 0;

    byte_t* _end = NULL;

	constexpr Forbidden_Array() {
        this->reserve(1);
    }

	constexpr explicit Forbidden_Array(size_t nreserve) {
        this->reserve(nreserve);
    }

	constexpr Forbidden_Array(const Forbidden_Array& src) {
        *this = src;
    }

	constexpr Forbidden_Array<type_t>& operator=(const Forbidden_Array<type_t>& src) {
        if (this == &src) {
            return *this;
        }

        this->reserve(src._buffer_size / type_size);
        
        memcpy(this->_buffer_head, src._buffer_head, _buffer_size);

        this->_end = this->_buffer_head + (src._end - src._buffer_head);

        return *this;
    }

	constexpr ~Forbidden_Array() {
        ST_FREE(_buffer_head, _buffer_size);
    }

	constexpr struct iterator
    {
        type_t* _p;
        constexpr iterator(type_t* p)
            :_p(p)
        {}
        constexpr iterator(byte_t* p)
            :_p((type_t*)p)
        {}

        constexpr iterator& operator++()
        {
            _p++;
            return *this;
        }

        constexpr iterator& operator--()
        {
            _p--;
            return *this;
        }

        constexpr type_t& operator*()
        {
            return *_p;
        }

        constexpr bool operator==(const iterator& b) const
        {
            return _p == b._p;
        }

        constexpr bool operator!=(const iterator& b) const
        {
            return !(*this == b);
        }

        
    };

	using const_iterator = const iterator;  // This is a simplistic way, in reality const_iterator would prevent modification of elements.
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr iterator begin() { return iterator(_buffer_head); }
    constexpr const_iterator begin() const { return const_iterator(_buffer_head); }
    constexpr const_iterator cbegin() const { return const_iterator(_buffer_head); }

    constexpr iterator end() { return iterator(_end); }
    constexpr const_iterator end() const { return const_iterator(_end); }
    constexpr const_iterator cend() const { return const_iterator(_end); }

    constexpr reverse_iterator rbegin() { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }

    constexpr reverse_iterator rend() { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    constexpr const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

	constexpr bool empty() const { return _end == _buffer_head; }

	constexpr size_t capacity() const { return _buffer_size / type_size; }

    constexpr void __resize_buffer(size_t n) {
        size_t new_buffer_size = n * type_size;
        byte_t* new_buffer = (byte_t*)ST_MEM(new_buffer_size);

        if (_buffer_head) {
            memcpy(new_buffer, _buffer_head, new_buffer_size > _buffer_size ? _buffer_size : new_buffer_size);

            _end = new_buffer + (_end - _buffer_head);

            ST_FREE(_buffer_head, _buffer_size);
        } else {
            _end = new_buffer;
        }

        _buffer_head = new_buffer;
        _buffer_size = type_size * n;
    }

	constexpr void reserve(size_t n) {
        if (n <= this->capacity()) return;

        __resize_buffer(n);
    }

	constexpr void resize(size_t n, const type_t& val = type_t()) {
        const size_t previous_num = this->size();
        if (n == previous_num) return;
        reserve(n);

        size_t diff = n - previous_num;

        for (size_t i = 0; i < diff; i++) {
            //*(((type_t*)_next) + i) = val;
            memcpy(_end + i * type_size, &val, type_size);
        }

        _end = _buffer_head + n * type_size;
    }

	constexpr size_t size() const {
        return (_end - _buffer_head) / type_size;
    }

	constexpr void shrink_to_fit() {
        if (_end == _buffer_head + _buffer_size) return;

        __resize_buffer(this->size());
    }

	constexpr void clear() {
        const size_t len = this->size();
        for (size_t i = 0; i < len; i++) {
            ((type_t*)_buffer_head)[i].~type_t();
        }
        _end = _buffer_head;
    }

    // Uses copy ctor, see emplace_back to avoid it
	constexpr type_t& push_back(const type_t& elem) {
        reserve(std::bit_ceil(this->size() + 1));

        ST_DEBUG_ASSERT(_end + type_size <= _buffer_head + _buffer_size);

        //*((type_t*)_next) = d;
        //memcpy(_end, &elem, type_size);
        new (_end) type_t(elem);

        _end += type_size;

        return *(((type_t*)_end) - 1);
    }

    template <typename ...args_t>
    constexpr type_t& emplace_back(args_t&&... args) {
        reserve(std::bit_ceil(this->size() + 1));

        new (_end) type_t(std::forward(args)...);

        _end += type_size;

        return *(((type_t*)_end) - 1);
    }

    constexpr type_t& insert(size_t n, const type_t& elem) {
        ST_DEBUG_ASSERT(n >= 0 && n <= this->size(), "Index out of bounds");

        reserve(std::bit_ceil(this->size() + 1));

        byte_t* insert_pos = _buffer_head + n * type_size;
        memmove(insert_pos + type_size, insert_pos, (_end - insert_pos));

        // Insert the new element
        //memcpy(insert_pos, &elem, type_size);
        new (insert_pos) type_t(elem);

        _end += type_size;
        return *(type_t*)insert_pos;
    }

    // Insert a single element at iterator position
    constexpr type_t& insert(iterator it, const type_t& d) {
        ST_DEBUG_ASSERT(it._p >= (type_t*)_buffer_head && it._p <= (type_t*)_end, "Iterator out of bounds");

        // Calculate the index where the new element will be inserted
        size_t n = it._p - (type_t*)_buffer_head;

        return insert(n, d);
    }

	constexpr void pop_back() {
        ST_DEBUG_ASSERT(_end >= _buffer_head, "Tried popping empty array");
        _end -= type_size;
        ((type_t*)_end)->~type_t();
    }

    // Erase single element at index
    constexpr void erase(size_t n) {
        ST_DEBUG_ASSERT(n >= 0 && n < this->size(), "Index out of bounds");

        ((type_t*)_buffer_head)[n].~type_t();

        byte_t* erase_pos = _buffer_head + n * type_size;
        memmove(erase_pos, erase_pos + type_size, (_end - erase_pos - type_size));

        _end -= type_size;
    }

    // Erase single element at iterator
    constexpr void erase(iterator it) {
        erase((it._p - (type_t*)_buffer_head));
    }

    // Erase several elements from n
    constexpr void erase(size_t n, size_t count) {
        ST_DEBUG_ASSERT(n >= 0 && (n + count) <= this->size(), "Index out of bounds");

        for (size_t i = 0; i < count; i++) {
            ((type_t*)_buffer_head)[n + i].~type_t();
        }

        byte_t* erase_pos = _buffer_head + n * type_size;
        memmove(erase_pos, erase_pos + count * type_size, (_end - erase_pos - count * type_size));

        _end -= (count * type_size);
    }

    // Erase several elements from begin iterator to end iterator
    constexpr void erase(iterator begin, iterator end) {
        erase((begin._p - (type_t*)_buffer_head), (end._p - begin._p));
    }

	constexpr type_t& at(size_t n) {
        ST_DEBUG_ASSERT(n >= 0 && n < this->size(), "Index out of bounds");
        return *(type_t*)(_buffer_head + n * type_size);
    }

	constexpr const type_t& at(size_t n) const {
        return this->at(n);
    }

	constexpr type_t& operator[](size_t n) {
        return this->at(n);
    }

	constexpr const type_t& operator[](size_t n) const {
        return (*this)[n];
    }

	constexpr type_t& front() {
        return this->at(0);
    }

	constexpr const type_t& front() const { return this->front(); }

	constexpr type_t& back() { return this->at(this->size() - 1); }

	constexpr const type_t& back() const { return this->back(); }

	constexpr type_t* data() { return (type_t*)_buffer_head; }
	constexpr const type_t* data() const { return this->data(); }
};

template <typename T>
using Array = typename std::conditional<
  std::is_trivially_copyable<T>::value,
  Forbidden_Array<T>,
  std::vector<T, Global_Allocator::STL_Global_Allocator<T>>
>::type;

template <typename T, typename U>
using Hash_Map = std::unordered_map<T, U, std::hash<T>, std::equal_to<T>, Global_Allocator::STL_Global_Allocator<std::pair<const T, U>>>;

template <typename T, typename U>
using Ordered_Map = std::map<T, U, std::less<T>, Global_Allocator::STL_Global_Allocator<std::pair<const T, U>>>;

template <typename T>
using Hash_Set = std::unordered_set<T, std::hash<T>, std::equal_to<T>, Global_Allocator::STL_Global_Allocator<T>>;

template <typename T>
using Ordered_Set = std::set<T, std::less<T>, Global_Allocator::STL_Global_Allocator<T>>;

using Dynamic_String = std::basic_string<char, std::char_traits<char>, Global_Allocator::STL_Global_Allocator<char>>;

template <typename T>
using Queue = std::queue<T, std::deque<T, Global_Allocator::STL_Global_Allocator<T>>>;

template <typename T>
using Deque = std::deque<T, Global_Allocator::STL_Global_Allocator<T>>;

template <typename T>
using Stack = std::stack<T, std::deque<T, Global_Allocator::STL_Global_Allocator<T>>>;