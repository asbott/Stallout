#pragma once //

#ifndef _ST_OS_WINDOWS
#error Windows-specifc header was included in non-windows build
#endif


NS_BEGIN(stallout);
NS_BEGIN(os);
NS_BEGIN(win32);

ST_API void assert_no_errors(bool uncaught, const char* file = "UNSPECIFIED", u32 line = 0, const char* action = "");

ST_API void clear_error();

NS_END(win32);
NS_END(os);
NS_END(stallout);


template<typename Func, typename Ret = std::invoke_result_t<Func>>
std::enable_if_t<!std::is_void_v<Ret>, Ret> win32_call_impl(Func&& func, const char* file, u32 line, const char* action) {
    //::os::win32::assert_no_errors(true, file, line);
    ::stallout::os::win32::clear_error();
    Ret ret = func();
    ::stallout::os::win32::assert_no_errors(false, file, line, action);
    return ret;
}

// Overload for void functions
template<typename Func, typename Ret = std::invoke_result_t<Func>>
std::enable_if_t<std::is_void_v<Ret>, void> win32_call_impl(Func&& func, const char* file, u32 line, const char* action) {
    //::os::win32::assert_no_errors(true);
    ::stallout::os::win32::clear_error();
    func();
    ::stallout::os::win32::assert_no_errors(false, file, line, action);
}

#ifndef _ST_DISABLE_ASSERTS
#define WIN32_CALL(x) win32_call_impl([&]() { return x; }, __FILE__, (u32)__LINE__, #x)
#else
#define WIN32_CALL(x) x
#endif