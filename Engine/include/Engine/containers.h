#pragma once

#include "Engine/memory.h"
#include "Engine/logger.h"

NS_BEGIN(engine);

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
    constexpr Forbidden_Array(std::initializer_list<type_t> items) {
        this->reserve(items.size());

        for (auto& item : items) {
            this->push_back(item);
        }
    }

	constexpr explicit Forbidden_Array(size_t n) {
        this->reserve(n);
        this->resize(n);
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

        if (n > previous_num) {
            size_t diff = n - previous_num;

            for (size_t i = 0; i < diff; i++) {
                //*(((type_t*)_next) + i) = val;
                memcpy(_end + i * type_size, &val, type_size);
            }
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
        ST_DEBUG_ASSERT(n >= 0 && n < this->size(), "Index out of bounds");
        return *(type_t*)(_buffer_head + n * type_size);
    }

	constexpr type_t& operator[](size_t n) {
        return this->at(n);
    }

	constexpr const type_t& operator[](size_t n) const {
        return this->at(n);
    }

	constexpr type_t& front() {
        return this->at(0);
    }

	constexpr const type_t& front() const { return this->front(); }

	constexpr type_t& back() { return this->at(this->size() - 1); }

	constexpr const type_t& back() const { return this->back(); }

	constexpr type_t* data() { return (type_t*)_buffer_head; }
	constexpr const type_t* data() const { return (type_t*)_buffer_head; }
};

template <typename T>
using STL_Array = std::vector<T, Global_Allocator::STL_Global_Allocator<T>>;

template <typename T>
using Array = typename std::conditional<
  std::is_trivially_copyable<T>::value,
  Forbidden_Array<T>,
  STL_Array<T>
>::type;



// General hash template
template<typename T>
struct GeneralHash {
    std::size_t operator()(const T& value) const {
        return std::hash<T>()(value);
    }
};

// Specialization for const char*
template<>
struct GeneralHash<const char*> {
    std::size_t operator()(const char* str) const {
        std::size_t hash = 0;
        while (*str) {
            hash = (hash << 5) - hash + *str++;
        }
        return hash;
    }
};

// General equality template
template<typename T>
struct GeneralEqual {
    bool operator()(const T& a, const T& b) const {
        return a == b;
    }
};

// Specialization for const char*
template<>
struct GeneralEqual<const char*> {
    bool operator()(const char* a, const char* b) const {
        return std::strcmp(a, b) == 0;
    }
};

// General less template
template<typename T>
struct GeneralLess {
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

// Specialization for const char*
template<>
struct GeneralLess<const char*> {
    bool operator()(const char* a, const char* b) const {
        return std::strcmp(a, b) < 0;
    }
};


template <typename T, typename U>
using Hash_Map = std::unordered_map<T, U, GeneralHash<T>, GeneralEqual<T>, Global_Allocator::STL_Global_Allocator<std::pair<const T, U>>>;

template <typename T, typename U>
using Ordered_Map = std::map<T, U, GeneralLess<T>, Global_Allocator::STL_Global_Allocator<std::pair<const T, U>>>;

template <typename T>
using Hash_Set = std::unordered_set<T, GeneralHash<T>, GeneralEqual<T>, Global_Allocator::STL_Global_Allocator<T>>;

template <typename T>
using Ordered_Set = std::set<T, GeneralLess<T>, Global_Allocator::STL_Global_Allocator<T>>;

template <typename T, typename alloc_t = Global_Allocator::STL_Global_Allocator<T>>
using Queue = std::queue<T, std::deque<T, alloc_t>>;

template <typename T>
using Deque = std::deque<T, Global_Allocator::STL_Global_Allocator<T>>;

template <typename T>
using Stack = std::stack<T, std::deque<T, Global_Allocator::STL_Global_Allocator<T>>>;


struct ST_API Bitset {
    Array<byte_t> bytes;

    Bitset(u16 hint_bits = 64) {
        bytes.resize((hint_bits + 7) / 8); 
    }

    bool operator ==(const Bitset& other) const {
        
        if (bytes.size() == 1 && other.bytes.size() == 1)
            return bytes[0] == other.bytes[0];
        else if (bytes.size() == 2 && other.bytes.size() == 2)
            return *(u16*)bytes.data() == *(u16*)other.bytes.data();
        else if (bytes.size() == 3 && other.bytes.size() == 3)
            return *(u16*)bytes.data() == *(u16*)other.bytes.data() &&
                bytes[2] == other.bytes[2];
        else if (bytes.size() == 4 && other.bytes.size() == 4)
            return *(u32*)bytes.data() == *(u32*)other.bytes.data();
        else if (bytes.size() == 5 && other.bytes.size() == 5)
            return *(u32*)bytes.data() == *(u32*)other.bytes.data() &&
                bytes[4] == other.bytes[4];
        else if (bytes.size() == 6 && other.bytes.size() == 6)
            return *(u32*)bytes.data() == *(u32*)other.bytes.data() &&
                *(u16*)(bytes.data() + 4) == *(u16*)(other.bytes.data() + 4);
        else if (bytes.size() == 7 && other.bytes.size() == 7)
            return *(u32*)bytes.data() == *(u32*)other.bytes.data() &&
                *(u16*)(bytes.data() + 4) == *(u16*)(other.bytes.data() + 4) &&
                bytes[6] == other.bytes[6];
        else if (bytes.size() == 8 && other.bytes.size() == 8)
            return *(u64*)bytes.data() == *(u64*)other.bytes.data();

        size_t min_size = std::min(bytes.size(), other.bytes.size());
        if (memcmp(bytes.data(), other.bytes.data(), min_size) != 0) {
            return false;
        }

        if (bytes.size() == other.bytes.size()) {
            return true;
        }

        const Array<byte_t>& larger_array = bytes.size() > other.bytes.size() ? bytes : other.bytes;

        for (size_t i = min_size; i < larger_array.size(); ++i) {
            if (larger_array[i] != 0) {
                return false;
            }
        }

        return true;
    }

    bool operator != (const Bitset& other) {
        return !(*this == other); 
    }

    void set(u16 bit) {
        if (bytes.size() * 8 <= bit) {
            bytes.resize(bit / 8 + 1); 
        }

        u16 nbyte = bit / 8;
        u16 nbit = bit % 8; 

        bytes[nbyte] |= (1 << nbit);
    }

    void unset(u16 bit) {
        if (bit / 8 >= bytes.size()) return;

        u16 nbyte = bit / 8;
        u16 nbit = bit % 8; 

        bytes[nbyte] &= ~(1 << nbit);
    }

    bool get(u16 bit) const {
        if (bit / 8 >= bytes.size()) return false;

        u16 nbyte = bit / 8;
        u16 nbit = bit % 8; 

        return bytes[nbyte] & (1 << nbit);
    }
};
NS_END(engine);