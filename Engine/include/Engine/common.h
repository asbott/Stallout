#ifdef _ST_OS_WINDOWS
	#ifdef _ST_EXPORT
		#define ST_API __declspec(dllexport)
	#else
		#define ST_API __declspec(dllimport)
	#endif
#else
	#define ST_API 
#endif

#ifdef _ST_OS_WINDOWS
	#define _AP_BREAK __debugbreak()
#elif defined (__GNUC__)
	#define _AP_BREAK __builtin_trap()
#else
	#define _AP_BREAK signal(SIGTRAP)
#endif

#ifndef _ST_DISABLE_ASSERTS

	#define st_assert(expr, ...) \
    if (!(expr)) { \
        log_critical("Assertion failed for expression '" #expr "'\nMessage:\n" __VA_ARGS__); _AP_BREAK; \
    }

#else

	#define st_assert(expr, ...)

#endif

#define BIT1  1
#define BIT2  2
#define BIT3  4
#define BIT4  8
#define BIT5  16
#define BIT6  32
#define BIT7  64
#define BIT8  128
#define BIT9  256
#define BIT10 512
#define BIT11 1024
#define BIT12 2048
#define BIT13 4096
#define BIT14 8192
#define BIT15 16384
#define BIT16 32768
#define BIT17 65536
#define BIT18 131072
#define BIT19 262144
#define BIT20 524288
#define BIT21 1048576
#define BIT22 2097152
#define BIT23 4194304
#define BIT24 8388608
#define BIT25 16777216
#define BIT26 33554432
#define BIT27 67108864
#define BIT28 134217728
#define BIT29 268435456
#define BIT30 536870912
#define BIT31 1073741824
#define BIT32 2147483648

#define BIT(n) (1 << n)

#ifdef _MSC_VER
	#define _st_force_inline __forceinline
#elif defined(__GNUC__) || defined (__MINGW32__) || defined(__MINGW64__)
	#define _st_force_inline __attribute__((always_inline)) inline
#elif defined(__clang__)
	#define _st_force_inline inline
#else
	#define _st_force_inline inline
	#warning _st_force_inline could not be defined for compiler.  
#endif

#ifdef _MSVC_LANG
	#define _ST_FUNC_SIG __FUNCTION__
#elif defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
	#define _ST_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
	#define _ST_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
	#define _ST_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
	#define _ST_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
	#define _ST_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
	#define _ST_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
	#define _ST_FUNC_SIG __func__
#else
	#define _ST_FUNC_SIG "[FUNC_SIG N/A]"
	#warning could not define function signature macro for this compiler
#endif

#define NOMINMAX

#define stringify(x) #x

#ifdef _ST_CONFIG_DEBUG
	#define debug_only(x) x
#else
	#define debug_only(x)
#endif

#define NS_BEGIN(x) namespace x {

#define NS_END(x) }

#ifdef _ST_OS_WINDOWS
	#define MODULE_FILE_EXTENSION "dll"
#elif defined(_OS_LINUX)
	#define MODULE_FILE_EXTENSION "so"
#endif

#define st_offsetof(st, m) ((size_t)(&((st *)0)->m))

#ifdef _ST_OS_WINDOWS
    #define export_function(ret) extern "C" __declspec(dllexport) ret __cdecl
#elif defined(_OS_LINUX)
    #define export_function(ret) extern "C" ret
#endif