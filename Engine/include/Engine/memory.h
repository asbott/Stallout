#pragma once
/*
    All allocators memset memory to 0

    Thread safety handled in sub allocators

    Allocator skeletons:



    void* allocate(size_t sz);
    void deallocate(void* p, size_t sz);

    bool contains(void* p);

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }
*/




//#define DISABLE_ALLOCATORS
//#define ST_ENABLE_MEMORY_DEBUG
//#define ST_ENABLE_MEMORY_DEBUG_EXTRA
//#define ST_ENABLE_MEMORY_TRACKING
//#define ST_ENABLE_MEMORY_LOGGING

#ifndef DISABLE_ALLOCATORS

    #ifndef ST_ENABLE_MEMORY_LOGGING
        #define ST_MEM(sz) (::engine::Global_Allocator::allocate(sz))
        #define ST_MEMF(sz, flags) (::engine::Global_Allocator::allocate(sz, flags))
        #define ST_FREE(p, sz) (::engine::Global_Allocator::deallocate(p, sz))
        #define ST_NEW(t, ...) new(::engine::Global_Allocator::allocate(sizeof(t))) t
    #else
        #define ST_MEM(sz) (::engine::Global_Allocator::allocate(sz, _ST_PASS_LOCATION))
        #define ST_MEMF(sz, flags) (::engine::Global_Allocator::allocate(sz, _ST_PASS_LOCATION, flags))
        #define ST_FREE(p, sz) (::engine::Global_Allocator::deallocate(p, sz, _ST_PASS_LOCATION))
        #define ST_NEW(t, ...) new(::engine::Global_Allocator::allocate(sizeof(t), _ST_PASS_LOCATION)) t
    #endif

    #define ST_DELETE(p) (::engine::Global_Allocator::deallocate_and_deconstruct(p))

#else

    #define ST_MEM(sz) (malloc(sz))
    #define ST_FREE(p, sz) {(free(p)); (void)(sz);}
    #define ST_NEW(st, ...) (new st(__VA_ARGS__))
    #define ST_DELETE(p) (delete p)

#endif

#define stnew ST_NEW
#define stdelete ST_DELETE

NS_BEGIN(engine);

typedef u8 Global_Alloc_Flag;
enum Global_Alloc_Flag_Impl {
    GLOBAL_ALLOC_FLAG_NONE = 0,
    GLOBAL_ALLOC_FLAG_ALIGN = BIT(1),
    GLOBAL_ALLOC_FLAG_FORBID_SMALL_ALLOC = BIT(2),
    GLOBAL_ALLOC_FLAG_STATIC = BIT(3),
    GLOBAL_ALLOC_FLAG_LARGE = BIT(4)
};



namespace Global_Allocator {

    ST_API void init(size_t preallocated = 1024 * 1000 * 1000);
    ST_API bool is_initialized();

#ifndef ST_ENABLE_MEMORY_LOGGING
    ST_API void* allocate(size_t sz, Global_Alloc_Flag flags = GLOBAL_ALLOC_FLAG_NONE);
    ST_API void deallocate(void* p, size_t sz);
#else
    ST_API void* allocate(size_t sz, _ST_LOCATION loc, Global_Alloc_Flag flags = GLOBAL_ALLOC_FLAG_NONE);
    ST_API void deallocate(void* p, size_t sz, _ST_LOCATION loc);
#endif

#ifdef ST_ENABLE_MEMORY_TRACKING
    ST_API size_t get_used_mem();
#endif

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = ST_MEM(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        
        ST_FREE(p, sizeof(type_t));
    }


    template <class T>
    class STL_Global_Allocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template <class U> struct rebind {
            typedef STL_Global_Allocator<U> other;
        };

        STL_Global_Allocator() noexcept {
            
        }

        template <class U>
        constexpr STL_Global_Allocator(const STL_Global_Allocator<U>&) noexcept {}

        T* allocate(size_type n) {
            if (n > max_size()) throw std::bad_alloc();
            auto p = static_cast<T*>(ST_MEM(n * sizeof(T)));
            if (!p) throw std::bad_alloc();
            return p;
        }

        void deallocate(T* p, size_type n) noexcept {
            ST_FREE(p, n * sizeof(T));
        }

        template <class U, class... Args>
        void construct(U* p, Args&&... args) {
            new((void*)p) U(std::forward<Args>(args)...);
        }

        template <class U>
        void destroy(U* p) {
            p->~U();
        }

        size_type max_size() const noexcept {
            return std::numeric_limits<size_type>::max() / sizeof(T);
        }
    };

    template <class T, class U>
    bool operator==(const STL_Global_Allocator<T>&, const STL_Global_Allocator<U>&) { return true; }

    template <class T, class U>
    bool operator!=(const STL_Global_Allocator<T>&, const STL_Global_Allocator<U>&) { return false; }
}

#ifdef _MSVC_LANG
#pragma warning(disable : 4251)
#endif

// 3rd level allocator, not thread safe
struct ST_API Block_Allocator {

    const size_t _block_size;
    const byte_t* _head;
    const byte_t* _tail;
    const bool _owns_buffer;
    size_t _total_size;
    byte_t* _pointer;
    mutable std::mutex _alloc_mutex;

    struct Free_Block {
        Free_Block* next = NULL;
    }* _next_free_block;

    // Mallocs buffer
    Block_Allocator(size_t total_size, size_t block_size);

    // Use given buffer instead of malloc
    Block_Allocator(void* buffer, size_t total_size, size_t block_size);

    Block_Allocator(const Block_Allocator& src) = delete;

    ~Block_Allocator();

    // Returns null if buffer overflow
    void* allocate(size_t sz);
    void deallocate(void* p, size_t sz);

    bool contains(void* p) const;

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }
};

struct ST_API Bin_Allocator {

    const size_t _number_of_bins, _bin_size;

    Block_Allocator* _blocks; // just for aligning the allocators as to avoid cache misses when just iterating them
    byte_t* _buffer; // Total memory shared between blocks

    Bin_Allocator(size_t number_of_bins, size_t bin_size);
    Bin_Allocator(const Bin_Allocator& other) = delete;
    ~Bin_Allocator();

    void* allocate(size_t sz);
    void deallocate(void* p, size_t sz);

    bool contains(void* p) const;

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }

};

// Dynamically creates bins
struct ST_API Growing_Bin_Allocator {

    const size_t _max_bin_size;
    const size_t _number_of_bins, _bin_size;

    std::vector<Bin_Allocator*> _bins;

    mutable std::mutex _mutex;

    Growing_Bin_Allocator(size_t number_of_bins, size_t bin_size);
    Growing_Bin_Allocator(const Growing_Bin_Allocator& other) = delete;
    //~Growing_Bin_Allocator();

    void* allocate(size_t sz);
    void deallocate(void* p, size_t sz);

    bool contains(void* p) const;

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }

};

// Not thread safe
struct ST_API Linear_Allocator {
    byte_t* _head, *_tail, *_next;   

    bool _is_buffer_owner;

    // TAKES ownership of buffer
    Linear_Allocator(size_t buffer_size);

    // Does NOT take ownership of buffer
    Linear_Allocator(void* existing_buffer, size_t buffer_size);

    ~Linear_Allocator();

    void* allocate(size_t sz);
    void deallocate(void* , size_t ) {}

    bool contains(void* p) { return p >= _head && p < _tail; }

    void reset();

    // Deletes current if responsible, but does not take responsibility for this new one
    void set_buffer(void* buffer, size_t buffer_size);

    // Takes responsibility for this one IF responsiblity for current
    void* swap_buffer(void* buffer, size_t buffer_size);

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    } 
};

// Minimum allocation size sizeof(Free_Node) (16 bytes)
struct First_Fit_Allocator {
    struct Free_Node {
        Free_Node* next;
        size_t size;
    } *_next_free;
    byte_t* _head, *_tail;

    First_Fit_Allocator(size_t buffer_size);
    ~First_Fit_Allocator();

    void* allocate(size_t sz);
    void deallocate(void* p, size_t sz);

    bool contains(void* p) const { return p >= _head && p < _tail; }

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }
};

NS_END(engine);