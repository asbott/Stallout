    #include "pch.h"

    #include "Stallout/memory.h"
    #include "Stallout/logger.h"
    #include "Stallout/timing.h"

    NS_BEGIN(stallout);

    namespace Global_Allocator {

    enum _Allocator_Type {
        ALLOC_TYPE_SMALL8,
        ALLOC_TYPE_SMALL16,
        ALLOC_TYPE_COMMON,
        ALLOC_TYPE_FAST32,
        ALLOC_TYPE_FAST64,
        ALLOC_TYPE_FAST128,
        ALLOC_TYPE_GROWING,
        ALLOC_TYPE_MALLOC
    };


    #ifdef ST_ENABLE_MEMORY_DEBUG
    void mark_garbage(void* vp, size_t sz) {
        byte_t* p = (byte_t*)vp;
        for (size_t i = 0; i < sz; i++) {
            byte_t& b = p[i];
            if (i % 2 == 0) {
                b = 72;
            } else {
                b = 199;
            }
        }
    }

    bool check_garbage(void* vp, size_t sz) {
        byte_t* p = (byte_t*)vp;

        byte_t last = 0;
        for (size_t i = 0; i < sz; i++) {
            byte_t& b = p[i];

            if (b != 72 && last != 72)   return false;
            if (b != 199 && last != 199) return false;

            last = b;
        }
        return true;
    }
    #endif

    #ifdef ST_ENABLE_MEMORY_META_DEBUG
    std::unordered_set<uintptr_t> _malloced_pointers;
    #endif

    constexpr u64 MAGIC_SIG_CONSTANT = 189473289017;
    constexpr u64 MAGIC_SIG_UNINIT_CONSTANT = 26147328017;
    struct Pointer_Meta {
        u64 magic_sig;
        size_t reqsz, storedsz;
    };

    struct Allocs {

        Allocs(size_t preallocated) 
            : small_allocator_8(Block_Allocator((size_t)(preallocated * 0.07), 8)),
            small_allocator_16(Block_Allocator((size_t)(preallocated * 0.07), 16)),
            small_allocator_32(Block_Allocator((size_t)(preallocated * 0.02), 32)),
            small_allocator_64(Block_Allocator((size_t)(preallocated * 0.02), 64)),
            small_allocator_128(Block_Allocator((size_t)(preallocated * 0.02), 128)),
            common_allocator(Free_List_Allocator((size_t)(preallocated * 0.8))),
                growing_allocator(Growing_Bin_Allocator(8, 1024 * 1000 * 8))
            {
            // Global allocator will handle thread safety
            small_allocator_8.enable_mutex_locking = false;
            small_allocator_16.enable_mutex_locking = false;
            small_allocator_32.enable_mutex_locking = false;
            small_allocator_64.enable_mutex_locking = false;
            small_allocator_128.enable_mutex_locking = false;
            common_allocator.enable_mutex_locking = false;
        }

        // TODO (2023-10-05): #memory #performance #threading
        // If all these small allocators were thread_local, they
        // would all fit into L1 cache per thread...........
        // Maybe even make them all share one contiguous buffer
        // per thread fitting into L1 (L2 if too small). All strings
        // and string operations under 128bytes would be extremely
        // fast.
        // It's probably even a non-trivial performance hit keeping
        // them like this because of false sharing.
        // This is however difficult to measure because it won't
        // really affect allocation time a lot, it's more about
        // cache efficiency so one would need to have an application
        // putting high stress on the allocation system to be able
        // to compare.

        // TODO (2023-11-01): #memory #performance
        // Should discard completely of the growing bin allocator
        // because it's trash. A growing array of free list
        // allocators should be fine.

        // TODO (2023-11-01): #memory #performance
        // Could maybe have two sets of small allocators
        // whereas one set is used when possible for all
        // regular allocations while one set is used only
        // for allocations with FAST_MEMORY flag. Reason
        // being that using block allocators up to 128
        // non-trivially decrease average allocation time
        // so it might be a good idea to make that a
        // standard BUT we also need some assurance that
        // allocations with FAST_MEMORY flags will indeed
        // be as fast as possible hence keeping a separate
        // set only used for such allocations.

        Block_Allocator small_allocator_8;
        Block_Allocator small_allocator_16;

        Block_Allocator small_allocator_32;
        Block_Allocator small_allocator_64;
        Block_Allocator small_allocator_128;

        Free_List_Allocator common_allocator;

        Growing_Bin_Allocator growing_allocator;

        Stats stats;

        std::mutex mutex;
        std::mutex tracking_mutex;
        std::mutex debug_mutex;
    }* allocs = NULL;


    // TODO #unfinished #feature #memory #performance
    // This is likely extremely situational as to what values
    // make for the best performance, so it should probably be
    // runtime values which can be modified by application.

    // For best avg allocation time, the max size should roughly
    // be the average allocation size. The num chance nodes is
    // simply trial and error to find the best value.
    #define MAX_NUM_CACHE_NODES 50
    #define MAX_CACHE_NODE_SIZE 128

    struct Thread_Cache_Free_Node {
        void* p = NULL;
        size_t size = 0;

        _Allocator_Type type;

        Thread_Cache_Free_Node* next = NULL;
    };
    // Should easily fit into inner cache
    thread_local Block_Allocator cache_node_allocator = Block_Allocator(MAX_NUM_CACHE_NODES * sizeof(Thread_Cache_Free_Node), sizeof(Thread_Cache_Free_Node));
    struct Thread_Cache {

        Thread_Cache_Free_Node* top = NULL;
        size_t cache_len = 0;

        bool push(void* p, size_t sz, _Allocator_Type type) {
            if (cache_len >= MAX_NUM_CACHE_NODES || sz > MAX_CACHE_NODE_SIZE) {
                return false;
            }

            Thread_Cache_Free_Node* node = cache_node_allocator.allocate_and_construct<Thread_Cache_Free_Node>();

            node->p = p;
            node->size = sz;
            node->type = type;

            node->next = top;
            top = node;

            cache_len++;
            return true;
        }

        void* pop(size_t* sz, _Allocator_Type* type) {
            if (*sz >= MAX_CACHE_NODE_SIZE) return NULL;

            Thread_Cache_Free_Node* last = NULL;
            Thread_Cache_Free_Node* current = top;
            void* match = NULL;

            while (current) {

                if (current->size >= *sz) {
                    match = current->p;
                    *sz = current->size;

                    if (last) {
                        last->next = current->next;
                    }

                    *type = current->type;

                    if (current == top) top = top->next;
                    cache_node_allocator.deallocate_and_deconstruct(current);

                    break;
                }                

                last = current;
                current = current->next;
            }

            if (match) cache_len--;
            return match;
        }

    };

    
    thread_local Thread_Cache thread_cache;


    void init(size_t preallocated) {
        allocs = new Allocs(preallocated);
        allocs->stats.preallocated = preallocated;
    }
    bool is_initialized() {
        return allocs != NULL;
    }

    #ifndef ST_ENABLE_MEMORY_LOGGING
    void* allocate(size_t sz, Global_Alloc_Flag flags) {
    #else
    void* allocate(size_t sz, _ST_LOCATION loc, Global_Alloc_Flag flags) {
    #endif

    if (!allocs) {
        void* p = malloc(sz + sizeof(u64));
        *((u64*)p) = MAGIC_SIG_UNINIT_CONSTANT;
        return ((u64*)p) + 1;
    }

    #ifdef ST_ENABLE_MEMORY_META_DEBUG
        sz += sizeof(Pointer_Meta);
    #endif

        size_t endsz = sz;

    #ifdef ST_ENABLE_MEMORY_TRACKING
        Timer timer;
    #endif

        if (!sz) return nullptr;

        _Allocator_Type type = ALLOC_TYPE_COMMON;

        
        bool used_cache = false; 
        void* mem = NULL;
    #ifdef ST_MEM_HEAP_ONLY
        mem = malloc(sz);
        type = ALLOC_TYPE_MALLOC;
        allocs->mutex.lock();
        if (true) goto DONE;
    #endif

        if (!(flags & GLOBAL_ALLOC_FLAG_STATIC) && !(flags & GLOBAL_ALLOC_FLAG_FORBID_THREAD_CACHE)) {
            mem = thread_cache.pop(&endsz, &type);
            if (mem) {
                used_cache = true;
                goto DONE;
            }
        }

        allocs->mutex.lock();

        if (!mem && !(flags & GLOBAL_ALLOC_FLAG_FORBID_SMALL_ALLOC)) {
            if (sz <= 8) {
                mem = allocs->small_allocator_8.allocate(sz, &endsz);
                type = ALLOC_TYPE_SMALL8;
            } else if (sz <= 16) {
                mem = allocs->small_allocator_16.allocate(sz, &endsz);
                type = ALLOC_TYPE_SMALL16;
            }
            if (mem) goto DONE;
        }

        // This stuff is a LOT of memory overhead but when
        // defaulting to allocating like this it does speed up
        // avg allocation time non-trivially (~0.02 mcs faster
        // only doing it up to 128 bytes)
        if (!mem/* && (flags & GLOBAL_ALLOC_FLAG_FAST_MEMORY)*/) {
            if (sz <= 32) {
                mem = allocs->small_allocator_32.allocate(sz, &endsz);
                type = ALLOC_TYPE_FAST32;
            } else if (sz <= 64) {
                mem = allocs->small_allocator_64.allocate(sz, &endsz);
                type = ALLOC_TYPE_FAST64;
            } else if (sz <= 128) {
                mem = allocs->small_allocator_128.allocate(sz, &endsz);
                type = ALLOC_TYPE_FAST128;
            }

            
            if (mem) goto DONE;
        }

        

        if (!mem && (flags & GLOBAL_ALLOC_FLAG_STATIC) && (flags & GLOBAL_ALLOC_FLAG_LARGE)) {
            mem = malloc(sz);
            type = ALLOC_TYPE_MALLOC;
            if (mem) goto DONE;
        }

        if (!mem) {
            
            mem = allocs->common_allocator.allocate(sz, &endsz);
            type = ALLOC_TYPE_COMMON;
            if (mem) goto DONE;
        }

        if (!mem && sz <= allocs->growing_allocator._max_bin_size) {
            mem = allocs->growing_allocator.allocate(sz, &endsz);
            type = ALLOC_TYPE_GROWING;
            if (mem) goto DONE;
        }

        if (!mem) {
            // TODO: Handle alloc failure depending on which allocator failed
            //       but still fallback like this if it can't be handled, or
            //       crash if the failure is unexpected. Also log a warning
            //       when for example a small allocator is full (once)

            mem = malloc(sz);
            type = ALLOC_TYPE_MALLOC;
            if (mem) goto DONE;
        }
    DONE:
    #ifdef ST_ENABLE_MEMORY_LOGGING
        log_debug("Global Memory Alloc\nAddress: {}\nRequested Bytes: {}\nAllocated Bytes: {}\nType: {}\nLocation: {}\nCached: {}", (u64)mem, sz, endsz, _alloc_type_str(type), loc, used_cache);

    #endif

        
    #ifdef ST_ENABLE_MEMORY_META_DEBUG

        if (type == ALLOC_TYPE_MALLOC) {
            _malloced_pointers.emplace((uintptr_t)((Pointer_Meta*)mem)+1);
        }

        ((Pointer_Meta*)mem)->magic_sig = MAGIC_SIG_CONSTANT;
        ((Pointer_Meta*)mem)->reqsz = sz - sizeof(Pointer_Meta);
        ((Pointer_Meta*)mem)->storedsz = endsz;
        
    #endif
        if (!used_cache || !mem) {
            allocs->mutex.unlock(); // DO NOT MOVE THIS DOWN
        }
        (void)type;(void)endsz;
    #ifdef ST_ENABLE_MEMORY_TRACKING

        auto time = timer.record();

        std::lock_guard tracking_lock(allocs->tracking_mutex);
        
        if(mem) {

            auto timems = time.get_microseconds() > 1.0 ? 0 : time.get_milliseconds(); 
            // Ignore large allocations
            if ((timems && endsz < 1024 * 10) || true) {
                if (used_cache) {
                    allocs->stats.cache_allocations++;
                    allocs->stats.cache_time += timems; 
                    allocs->stats.cache_amount += endsz;
                } else {
                    switch (type) {
                        case ALLOC_TYPE_SMALL8: 
                            allocs->stats.small_allocations++;
                            allocs->stats.small_time += timems; 
                            allocs->stats.small_amount += endsz;
                            break;
                        case ALLOC_TYPE_SMALL16: 
                            allocs->stats.small_allocations++; 
                            allocs->stats.small_time += timems; 
                            allocs->stats.small_amount += endsz;
                            break;
                        case ALLOC_TYPE_FAST32: case ALLOC_TYPE_FAST64: case ALLOC_TYPE_FAST128: 
                            allocs->stats.fast_allocations++; 
                            allocs->stats.fast_time += timems; 
                            allocs->stats.fast_amount += endsz;
                            break;
                        case ALLOC_TYPE_COMMON: 
                            allocs->stats.common_allocations++; 
                            allocs->stats.common_time += timems; 
                            allocs->stats.common_amount += endsz;
                            break;
                        case ALLOC_TYPE_GROWING: 
                            allocs->stats.growing_allocations++; 
                            allocs->stats.growing_time += timems; 
                            allocs->stats.growing_amount += endsz;
                            break;
                        case ALLOC_TYPE_MALLOC: 
                            allocs->stats.heap_allocations++; 
                            allocs->stats.heap_time += timems; 
                            allocs->stats.heap_amount += endsz;
                            break;
                        default: INTENTIONAL_CRASH("Unhandled enum"); break;
                    }
                }
            }
        }

        allocs->stats.tracked_allocated += endsz;

    #endif

    #ifdef ST_ENABLE_MEMORY_DEBUG
    #ifdef ST_ENABLE_MEMORY_META_DEBUG
        auto* pm = ((Pointer_Meta*)mem);
        void* p = ((Pointer_Meta*)mem) + 1; 
        memset(p, 0, pm->reqsz);
    #else
        memset(mem, 1, sz);
    #endif
    #endif

    #ifdef ST_ENABLE_MEMORY_META_DEBUG
        return ((Pointer_Meta*)mem)+1;
    #else
        return mem;
    #endif    
    }

    

    #ifndef ST_ENABLE_MEMORY_LOGGING
    void deallocate(void* p, size_t sz) {
    #else
    void deallocate(void* p, size_t sz, _ST_LOCATION loc) {
    #endif
        
        if (*(((u64*)p) - 1) == MAGIC_SIG_UNINIT_CONSTANT) {
            free((((u64*)p) - 1));
    #ifdef ST_ENABLE_MEMORY_LOGGING
            log_debug("Freeing pointer allocated before init: {}", (uintptr_t)p);
    #endif
            return;
        }
        


        size_t endsz = sz;
    #ifdef ST_ENABLE_MEMORY_META_DEBUG
        Pointer_Meta* meta = ((Pointer_Meta*)p)-1;

        const char* msg = "";

        if (meta->magic_sig != MAGIC_SIG_CONSTANT) {
            // Either memory was overwritten or it was not allocated here

            if (!allocs->small_allocator_8.contains(p) 
                && !allocs->small_allocator_16.contains(p)
                && !allocs->small_allocator_32.contains(p)
                && !allocs->small_allocator_64.contains(p)
                && !allocs->small_allocator_128.contains(p)
                && !allocs->common_allocator.contains(p)
                && !allocs->growing_allocator.contains(p)
                && !_malloced_pointers.contains((uintptr_t)p)) 
                {
                msg = "Pointer not allocated by global allocator";
            } else {
                msg = "Memory has been corrupt; likely unallocated memory has been written";
            }

            INTENTIONAL_CRASH("Deallocation error: {}", msg);
        }

        ST_DEBUG_ASSERT(meta->reqsz == sz, "Bad deallocation: Pointer was allocated as {}, but {} was passed", meta->reqsz, sz);

        p = meta;
        sz += sizeof(Pointer_Meta);

        // Need to store this here because after deallocation, memory in meta may be overwritten.
        size_t stored_sz = meta->storedsz;
        endsz = stored_sz;
    #endif

#ifdef ST_ENABLE_MEMORY_DEBUG

        ST_DEBUG_ASSERT(!check_garbage(p, sz), "Deallocating pointer to garbage");

        mark_garbage(p, sz);

#endif
        
        _Allocator_Type dealloc_type = ALLOC_TYPE_COMMON;
        ST_DEBUG_ASSERT(p != nullptr, "Cannot deallocate null");

        auto last_thread_cache_top = thread_cache.top;
        (void)last_thread_cache_top;

        // TODO (2023-10-07): #performance #memory
        // This might cause an indirection which is bad enough
        // to have a significant impact on performance.
        thread_local void(*_alloc_fn)(void*, size_t) = 0;
        
        if (allocs->small_allocator_8.contains(p)) {
            dealloc_type = ALLOC_TYPE_SMALL8;
            endsz = allocs->small_allocator_8.align(sz);
            _alloc_fn = [](void* p, size_t sz) { allocs->small_allocator_8.deallocate(p, sz); };
        } else if (allocs->small_allocator_16.contains(p)) {
            dealloc_type = ALLOC_TYPE_SMALL16;
            endsz = allocs->small_allocator_16.align(sz);
            _alloc_fn = [](void* p, size_t sz) { allocs->small_allocator_16.deallocate(p, sz); };
        } else if (allocs->small_allocator_32.contains(p)) {
            dealloc_type = ALLOC_TYPE_FAST32;
            endsz = allocs->small_allocator_32.align(sz);
            _alloc_fn = [](void* p, size_t sz) { allocs->small_allocator_32.deallocate(p, sz); };
        } else if (allocs->small_allocator_64.contains(p)) {
            dealloc_type = ALLOC_TYPE_FAST64;
            endsz = allocs->small_allocator_64.align(sz);
            _alloc_fn = [](void* p, size_t sz) { allocs->small_allocator_64.deallocate(p, sz); };
        } else if (allocs->small_allocator_128.contains(p)) {
            dealloc_type = ALLOC_TYPE_FAST128;
            endsz = allocs->small_allocator_128.align(sz);
            _alloc_fn = [](void* p, size_t sz) { allocs->small_allocator_128.deallocate(p, sz); };
        } else if (allocs->common_allocator.contains(p)) {
            dealloc_type = ALLOC_TYPE_COMMON;
            endsz = allocs->common_allocator.align(sz);
            _alloc_fn = [](void* p, size_t sz) { allocs->common_allocator.deallocate(p, sz); };
        } else if (allocs->growing_allocator.contains(p)) {
            dealloc_type = ALLOC_TYPE_GROWING;
            endsz = allocs->growing_allocator.align(sz);
            _alloc_fn = [](void* p, size_t sz) { allocs->growing_allocator.deallocate(p, sz); };
        } else {
            dealloc_type = ALLOC_TYPE_MALLOC;
    #ifdef ST_ENABLE_MEMORY_META_DEBUG
            _malloced_pointers.erase((uintptr_t)p);
    #endif
            _alloc_fn = [](void* p, size_t sz) { free(p); (void)sz; };
        }

        ST_DEBUG_ASSERT(endsz >= sz, "Internal memory error: aligned to lower size");

        // Beyond this point the meta part is unusable because 
        // likely overwritten with garbage from deallocation.

        if (!thread_cache.push(p, endsz, dealloc_type)) {
            std::lock_guard l(allocs->mutex);
            _alloc_fn(p, endsz);
        }
        (void)dealloc_type;

    #ifdef ST_ENABLE_MEMORY_META_DEBUG
        ST_DEBUG_ASSERT(stored_sz == endsz, "Internal memory error: Stored size mismatch with deallocated size");
    #endif


    #ifdef ST_ENABLE_MEMORY_TRACKING
        std::lock_guard tracking_lock(allocs->tracking_mutex);
        allocs->stats.tracked_deallocated += endsz;

        // Cached allocation ? 
        if (last_thread_cache_top != thread_cache.top) {
            allocs->stats.cache_freed += endsz;
        } else {
            switch (dealloc_type) {
                case ALLOC_TYPE_SMALL8: 
                    allocs->stats.small_freed += endsz;
                    break;
                case ALLOC_TYPE_SMALL16: 
                    allocs->stats.small_freed += endsz;
                    break;
                case ALLOC_TYPE_FAST32: case ALLOC_TYPE_FAST64: case ALLOC_TYPE_FAST128: 
                    allocs->stats.fast_freed += endsz;
                    break;
                case ALLOC_TYPE_COMMON: 
                    allocs->stats.common_freed += endsz;
                    break;
                case ALLOC_TYPE_GROWING: 
                    allocs->stats.growing_freed += endsz;
                    break;
                case ALLOC_TYPE_MALLOC: 
                    allocs->stats.heap_freed += endsz;
                    break;
                default: INTENTIONAL_CRASH("Unhandled enum"); break;
            }
        }
    #endif

    }

    const Stats& get_stats() {
        return allocs->stats;
    }

    void reset_stats() {
        allocs->stats = Stats();
    }
    }



    Block_Allocator::Block_Allocator(size_t total_size, size_t block_size) 
    : _block_size(block_size), _owns_buffer(true) {
    auto aligned_size = ALIGN(total_size, block_size);
    _total_size = aligned_size;
    _head = (byte_t*)malloc(aligned_size);
    _tail = (_head) + aligned_size;

    _next_free_block = NULL;
    _pointer = (byte_t*)_head;
    }
    Block_Allocator::Block_Allocator(void* buffer, size_t total_size, size_t block_size) 
    : _block_size(block_size), _owns_buffer(false) {
    auto aligned_size = ALIGN(total_size, block_size);
    _total_size = aligned_size;
    ST_DEBUG_ASSERT(aligned_size == total_size, "When providing block allocator with preallocated buffer, its size must be aligned to the block size (total_size is {}, aligns to {})", total_size, aligned_size);
    _head = (byte_t*)buffer;
    _tail = (_head) + aligned_size;

    _next_free_block = NULL;
    _pointer = (byte_t*)_head;
    }

    Block_Allocator::~Block_Allocator() {
    if(_owns_buffer) free((void*)_head);
    }

    size_t Block_Allocator::align(size_t ) const {
    return _block_size;
    }
    void* Block_Allocator::allocate(size_t sz, size_t* aligned_sz) {
    (void)sz;
    ST_DEBUG_ASSERT(sz <= _block_size, "Allocation size {} exceeds block size {}", sz, _block_size);

    if (aligned_sz ) *aligned_sz = _block_size;

    void* p = NULL;
    {
        // TODO (2023-12-05): #memory #design #refactor
        // Should first check if _pointer is not _tail
        // and allocate from there, or at least have the
        // option to do so with the benefit of sequential
        // allocations being contiguous.
        if (enable_mutex_locking) _alloc_mutex.lock();
        if (_next_free_block) {
            p = _next_free_block;
            _next_free_block = _next_free_block->next; // If you crash here it's likely you've overwritten memory
        } else {
            if ((_pointer + _block_size) > _tail) {
                if (enable_mutex_locking) _alloc_mutex.unlock();
                return NULL;
            }

            p = _pointer;
            _pointer += _block_size;
        }
    }
    if (enable_mutex_locking) _alloc_mutex.unlock();
    return p;
    }
    void Block_Allocator::deallocate(void* p, size_t sz, size_t* aligned_sz) {
    (void)sz;
    ST_DEBUG_ASSERT(p != nullptr, "Cannot deallocate null");
    ST_DEBUG_ASSERT(sz <= _block_size, "Size {} exceeds block size {}; cannot deallocate", sz, _block_size);
    ST_DEBUG_ASSERT(((byte_t*)p) >= _head && ((byte_t*)p) < _tail, "Tried deallocating pointer in non-owner allocator");

    auto* new_free_block = (Free_Block*)p;

    if (aligned_sz) *aligned_sz = _block_size;

    if (enable_mutex_locking) _alloc_mutex.lock();

    new_free_block->next = _next_free_block;
    _next_free_block = new_free_block;

    if (enable_mutex_locking) _alloc_mutex.unlock();
    }

    bool Block_Allocator::contains(void* p) const {
    return (byte_t*)p >= _head && (byte_t*)p < _tail;
    }


    Bin_Allocator::Bin_Allocator(size_t number_of_bins, size_t bin_size) 
    : _number_of_bins(number_of_bins), _bin_size(bin_size) {

    // TODO: This allocation likely slows down a lot
    _blocks = (Block_Allocator*)malloc(number_of_bins * sizeof(Block_Allocator));

    // Find what the total size will be for all blocks collectively
    size_t total_size = 0;
    for (size_t i = 0; i < number_of_bins; i++) {
        auto align = (size_t)pow(2, i + 3);
        total_size += ALIGN(bin_size, align);
    }

    // Allocate memory for all blocks in one buffer
    // TODO: This allocation likely slows down a lot
    _buffer = (byte_t*)malloc(total_size);
    byte_t* next = _buffer;

    // Initialize the block allocators and give them their respective
    // chunk of memory from the buffer
    for (size_t i = 0; i < number_of_bins; i++) {
        auto align = (size_t)pow(2, i + 3);
        auto bin_size_aligned = ALIGN(bin_size, align);
        new(&_blocks[i]) Block_Allocator(next, bin_size_aligned, align);
        next += bin_size_aligned;
    }
    
    }
    Bin_Allocator::~Bin_Allocator() {
    for (size_t i = 0; i < _number_of_bins; i++) {
        _blocks[i].~Block_Allocator();
    }
    free(_blocks);
    free(_buffer);
    }

    size_t Bin_Allocator::align(size_t sz) const {
    for (size_t i = 0; i < _number_of_bins; i++) {
        if (sz <= _blocks[i]._block_size) {
            return _blocks[i]._block_size;
        }
    }
    return sz;
    }
    void* Bin_Allocator::allocate(size_t sz, size_t* aligned_sz) {
    if (!sz) return nullptr;
    if (sz <= 8) {
        sz = 8;
    }

    for (size_t i = 0; i < _number_of_bins; i++) {
        if (sz <= _blocks[i]._block_size) {
            void* p = _blocks[i].allocate(sz, aligned_sz);
            if (p) return p; // If null, the bin is full
        }
    }

    

    return NULL;
    }
    void Bin_Allocator::deallocate(void* p, size_t sz, size_t* aligned_sz) {
    ST_DEBUG_ASSERT(p != nullptr, "Cannot deallocate null");
    for (size_t i = 0; i < _number_of_bins; i++) {

        if (_blocks[i].contains(p)) {
            _blocks[i].deallocate(p, sz, aligned_sz);
            return;
        }
    }

    ST_DEBUG_ASSERT(false, "Tried deallocating in non-owner allocator", sz);
    }

    bool Bin_Allocator::contains(void* p) const {
    for (size_t i = 0; i < _number_of_bins; i++) {
        if (_blocks[i].contains(p)) return true;
    }
    return false;
    }








    Growing_Bin_Allocator::Growing_Bin_Allocator(size_t number_of_bins, size_t bin_size)
    : _number_of_bins(number_of_bins), _bin_size(bin_size), _max_bin_size((size_t)pow(2, number_of_bins - 1 + 3))
    {
    // TODO: vector copies so cant do just raw, fix
    _bins.push_back(new Bin_Allocator(_number_of_bins, _bin_size));
    }

    size_t Growing_Bin_Allocator::align(size_t sz) const {
    for(auto& bin : _bins) {
        if (bin->align(sz) != sz) return bin->align(sz);
    }
    return sz;
    }
    void* Growing_Bin_Allocator::allocate(size_t sz, size_t* aligned_sz) {
    if (!sz) return nullptr;
    void* p = NULL;
    {
        std::lock_guard<std::mutex> lock(_mutex);

        bool any = false;
        // TODO: BIG bottleneck O(n)
        for(auto& bin : _bins) {
            // TODO: probably faster to manually check if allocation is possible
            p = bin->allocate(sz, aligned_sz);
            if (p) {
                any = true;
                break;
            }
        }

        if (!any) {
            // TODO: This allocation likely slows down a lot
            _bins.push_back(new Bin_Allocator(_number_of_bins, _bin_size));
            p = _bins.back()->allocate(sz, aligned_sz);
        } 
    }
    
    return p;
    }

    void Growing_Bin_Allocator::deallocate(void* p, size_t sz, size_t* aligned_sz) {
    std::lock_guard<std::mutex> lock(_mutex);
    // TODO: BIG bottleneck O(n)
    for(auto& bin : _bins) {
        if(bin->contains(p)) {
            bin->deallocate(p, sz, aligned_sz);
            return;
        }
    }
    }

    bool Growing_Bin_Allocator::contains(void* p) const {
    std::lock_guard<std::mutex> lock(_mutex);
    for(const auto& bin : _bins) {
        if(bin->contains(p)) return true;
    }
    return false;
    }

    Linear_Allocator::Linear_Allocator(size_t buffer_size)
    : _is_buffer_owner(true) {
    _head = (byte_t*)ST_MEM(buffer_size);
    _tail = _head + buffer_size;
    _next = _head;
    }
    Linear_Allocator::Linear_Allocator(void* existing_buffer, size_t buffer_size) 
    : _is_buffer_owner(false){
    set_buffer(existing_buffer, buffer_size);
    }

    Linear_Allocator::~Linear_Allocator() {
    if (_is_buffer_owner) ST_FREE(_head, (size_t)(_tail - _head));
    }

    size_t Linear_Allocator::align(size_t sz) const {
    return sz;
    }

    void* Linear_Allocator::allocate(size_t sz, size_t* aligned_sz) {
    ST_DEBUG_ASSERT(_next + sz <= _tail, "Linear allocator overflow");
    
    void* p = _next;
    
    _next += sz;
    if (aligned_sz ) *aligned_sz = sz;

    return p;
    }

    void Linear_Allocator::reset() {
    _next = _head;
    }

    void Linear_Allocator::set_buffer(void* buffer, size_t buffer_size) {
    ST_DEBUG_ASSERT(buffer);
    if (_is_buffer_owner) ST_FREE(_head, (size_t)(_tail-_head));
    
    _head = (byte_t*)buffer;
    _tail = _head + buffer_size;
    _next = _head;

    _is_buffer_owner = false;
    }

    void* Linear_Allocator::swap_buffer(void* buffer, size_t buffer_size) {
    ST_DEBUG_ASSERT(buffer);
    void* old_buffer = _head;

    _head = (byte_t*)buffer;
    _tail = _head + buffer_size;
    _next = _head;

    return old_buffer;
    }



    Free_List_Allocator::Free_List_Allocator(size_t buffer_size, Fit_Mode fit_mode) {
    _head = (byte_t*)malloc(buffer_size);
    _tail = _head + buffer_size;

    _next_free = (Free_Node*)_head;
    _next_free->next = NULL;
    _next_free->size = buffer_size;

    owner = true;

    _fit_mode = fit_mode;
    }
    Free_List_Allocator::Free_List_Allocator(void* buffer, size_t buffer_size, Fit_Mode fit_mode) {
    _head = (byte_t*)buffer;
    _tail = _head + buffer_size;

    _next_free = (Free_Node*)_head;
    _next_free->next = NULL;
    _next_free->size = buffer_size;

    owner = false;

    _fit_mode = fit_mode;
    }
    Free_List_Allocator::~Free_List_Allocator() {
    if (owner) free(_head);
    }

    size_t Free_List_Allocator::align(size_t sz) const {
    return ALIGN(sz, sizeof(Free_Node));
    }
    void* Free_List_Allocator::allocate(size_t sz, size_t* aligned_sz) {

    if (enable_mutex_locking) _alloc_mutex.lock();

    Free_Node* next = _next_free;
    Free_Node* prev = NULL;
    Free_Node* best_fit_node = NULL;
    Free_Node* best_fit_prev = NULL;
    size_t best_fit_size = SIZE_MAX;  // Start with a maximum size for comparison

    sz = ALIGN(sz, sizeof(Free_Node));

    if (aligned_sz ) *aligned_sz = sz;

    while (next) {
        if (sz <= next->size) {
            if (_fit_mode == FIT_MODE_FIRST) {
                void* p = (void*)next;

                if (sz < next->size) {
                    // Create a new free node in the remaining space
                    Free_Node* new_node = (Free_Node*)((byte_t*)next + sz);
                    new_node->size = next->size - sz;
                    new_node->next = next->next;

                    next->size = sz;
                    next->next = new_node;
                }

                // Update the previous node's next, or the head of the list
                if (prev) {
                    prev->next = next->next;
                } else {
                    _next_free = next->next;
                }

                if (enable_mutex_locking) _alloc_mutex.unlock();
                return p;
            }
            else if (_fit_mode == FIT_MODE_FIT) {
                if (next->size < best_fit_size) {
                    best_fit_size = next->size;
                    best_fit_node = next;
                    best_fit_prev = prev;
                }
            }
        }

        prev = next;
        next = next->next;
    }

    // If using FIT_MODE_FIT and a suitable block was found
    if (_fit_mode == FIT_MODE_FIT && best_fit_node) {
        void* p = (void*)best_fit_node;

        if (sz < best_fit_node->size) {
            // Create a new free node in the remaining space
            Free_Node* new_node = (Free_Node*)((byte_t*)best_fit_node + sz);
            new_node->size = best_fit_node->size - sz;
            new_node->next = best_fit_node->next;

            best_fit_node->size = sz;
            best_fit_node->next = new_node;
        }

        // Update the previous node's next, or the head of the list
        if (best_fit_prev) {
            best_fit_prev->next = best_fit_node->next;
        } else {
            _next_free = best_fit_node->next;
        }

        if (enable_mutex_locking) _alloc_mutex.unlock();
        return p;
    }

    if (enable_mutex_locking) _alloc_mutex.unlock();
    
    return NULL; 
    }
    void Free_List_Allocator::deallocate(void* p, size_t sz, size_t* aligned_sz) {
        ST_DEBUG_ASSERT(this->contains(p), "Pointer does not belong in this allocator");

        if (enable_mutex_locking) _alloc_mutex.lock();
        Free_Node* new_node = (Free_Node*)p;
        new_node->size = sz;

        Free_Node* prev = NULL;
        Free_Node* next = _next_free;

        sz = ALIGN(sz, sizeof(Free_Node));
        if (aligned_sz) *aligned_sz = sz;

        // Find the correct position to insert the new free node
        while (next && (uintptr_t)next < (uintptr_t)new_node) {
            prev = next;
            next = next->next;
        }

        // Insert and try to coalesce with following block
        new_node->next = next;
        if ((byte_t*)new_node + new_node->size == (byte_t*)next) {
            new_node->size += next->size;
            new_node->next = next->next;
        }

        // Try to coalesce with previous block
        if (prev && (byte_t*)prev + prev->size == (byte_t*)new_node) {
            prev->size += new_node->size;
            prev->next = new_node->next;
        } else if (prev) {
            prev->next = new_node;
        } else {  // If prev is NULL, new_node becomes the new head
            _next_free = new_node;
        }

        if (enable_mutex_locking) _alloc_mutex.unlock();
    }

    NS_END(stallout);