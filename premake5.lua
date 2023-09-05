

function stallout_project(name) 
    project (name)
    language   "C++"
    warnings   "Extra"
    cppdialect "C++20"
    targetdir  ("bin/%{cfg.buildcfg}/%{wks.name}/%{prj.name}")
    objdir     ("bin/%{cfg.buildcfg}/%{wks.name}/%{prj.name}/int")
    vectorextensions "SSE4.1"
    location "%{prj.name}"
    includedirs {
        "%{prj.location}/include",
        "deps/mz",
        "deps/imgui",
        "deps/spdlog/include",
    }
    files { "%{prj.location}/src/**.cpp", "%{prj.location}/include/**.h" }
    flags {
        "FatalWarnings",
        "MultiProcessorCompile"
    }

    pchheader "pch.h"
    pchsource "%{prj.location}/src/pch.cpp"
end

workspace "Stallout"  
    configurations { "Debug", "Test", "Release" } 

    architecture "x64"
    systemversion "latest"

    startproject "Launcher"

    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "_CRTLDBG_REPORT_FLAG",
        "FMT_HEADER_ONLY"
    }

    

    filter "configurations:Debug*"
        defines {"_ST_CONFIG_DEBUG", "_ST_ENABLE_GL_DEBUG_CONTEXT"}
        runtime "Debug"
        symbols "On"
        floatingpoint "Default"

    filter "configurations:Test*"
        defines  "_ST_CONFIG_TEST"
        runtime  "Release"
        optimize "Debug"
        floatingpoint "Default"

    filter "configurations:Release*"
        defines  { "_ST_CONFIG_RELEASE", "_ST_DISABLE_ASSERTS", "NDEBUG" }
        runtime  "Release"
        symbols  "Off"
        optimize "Speed"
        floatingpoint "Fast"

    filter "system:windows"
        defines { "_ST_OS_WINDOWS" }
        exceptionhandling "SEH"

    filter "system:linux"
        pic "On"
        defines { "_ST_OS_LINUX" }

    filter "action:vs*"
        buildoptions { "/wd4201", "/wd26495" }

    stallout_project "Launcher"
        kind "ConsoleApp"

        includedirs {
            "Engine/include"
        }

        links {
            "Engine"
        }

        postbuildcommands {
            "{COPY} %{wks.location}deps/openal/lib/%{cfg.buildcfg}/OpenAL32.dll %{wks.location}bin/%{cfg.buildcfg}/%{wks.name}/Launcher/"
        }

    stallout_project "Engine"
        kind "SharedLib"

        postbuildcommands {
            "{COPY} %{cfg.buildtarget.relpath} %{wks.location}bin/%{cfg.buildcfg}/%{wks.name}/Launcher//"
        }

        defines {
            "GLFW_STATIC",
            "GLAD_STATIC",
            "_ST_EXPORT"
        }

        includedirs {
            "Engine/include/Engine",
            "deps/glad/include",
            "deps/glfw/include",
            
            "deps/stb",
            "deps/openal/repo/include"
        }

        links {
            "dearimgui",
            "glfw",
            "glad",
            "OpenAL32"
        }

        removefiles {
            "%{prj.location}/include/os/**",
            "%{prj.location}/src/os/**"
        }

        files {
            "%{prj.location}/include/os/*.h",
            "%{prj.location}/src/os/*.cpp"
        }

        filter "system:windows"
            links {
                "opengl32",
                "glu32",
                "gdi32"
            }

            files {
                "%{prj.location}/include/os/windows/*.h",
                "%{prj.location}/src/os/windows/*.cpp"
            }

        filter "system:linux"
            pic "On"

            links {
                "GL",
                "GLU",
                "X11",
                "Xxf86vm",
                "Xrandr",
                "pthread",
                "Xi",
                "dl",
                "stdc++fs",
            }

        
        filter "configurations:Debug*"
            libdirs {
                "deps/openal/lib/Debug"
            }
        
        filter "configurations:Test*"
            libdirs {
                "deps/openal/lib/Test"
            }
        filter "configurations:Release*"
            libdirs {
                "deps/openal/lib/Release"
            }
        

    stallout_project "Sandbox"
        kind "SharedLib"

        includedirs "Engine/include"

        links {
            "Engine"
        }

        postbuildcommands {
            "{COPY} %{cfg.buildtarget.relpath} %{wks.location}bin/%{cfg.buildcfg}/%{wks.name}/Launcher/"
        }

    project "glfw"
        location   "deps/glfw"
        kind       "StaticLib"
        language   "C"
        warnings   "Off"
        targetdir  ("bin/%{cfg.buildcfg}/deps/%{prj.name}")
        objdir     ("bin/%{cfg.buildcfg}/deps/%{prj.name}/int")

        files {
            "deps/glfw/include/glfw/glfw3.h",
            "deps/glfw/include/glfw/glfw3native.h",
            "deps/glfw/src/glfw_config.h",
            "deps/glfw/src/context.c",
            "deps/glfw/src/init.c",
            "deps/glfw/src/input.c",
            "deps/glfw/src/monitor.c",
            "deps/glfw/src/vulkan.c",
            "deps/glfw/src/window.c"
        }

        filter "system:windows"
            files {
                "deps/glfw/src/win32_init.c",
                "deps/glfw/src/win32_joystick.c",
                "deps/glfw/src/win32_monitor.c",
                "deps/glfw/src/win32_time.c",
                "deps/glfw/src/win32_thread.c",
                "deps/glfw/src/win32_window.c",
                "deps/glfw/src/wgl_context.c",
                "deps/glfw/src/egl_context.c",
                "deps/glfw/src/osmesa_context.c"
            }
        
            defines { 
                "_GLFW_WIN32"
            }

        filter "system:linux"
            pic "On"

            files {
                "deps/glfw/src/x11_init.c",
                "deps/glfw/src/x11_monitor.c",
                "deps/glfw/src/x11_window.c",
                "deps/glfw/src/xkb_unicode.c",
                "deps/glfw/src/posix_time.c",
                "deps/glfw/src/posix_thread.c",
                "deps/glfw/src/glx_context.c",
                "deps/glfw/src/egl_context.c",
                "deps/glfw/src/osmesa_context.c",
                "deps/glfw/src/linux_joystick.c"
            }

            defines {
                "_GLFW_X11"
            }

    project "glad"
        location   "deps/glad"
        kind       "StaticLib"
        language   "C"
        warnings   "Off"
        targetdir  ("bin/%{cfg.buildcfg}/deps/%{prj.name}")
        objdir     ("bin/%{cfg.buildcfg}/deps/%{prj.name}/int")

        files {
            "deps/glad/include/glad/glad.h",
            "deps/glad/include/KHR/khrplatform.h",
            "deps/glad/src/glad.c"
        }
    
        includedirs { "deps/glad/include" }

    project "dearimgui"
        location   "deps/dearimgui"
        kind       "StaticLib"
        language   "C++"
        warnings   "Off"
        targetdir  ("bin/%{cfg.buildcfg}/deps/%{prj.name}")
        objdir     ("bin/%{cfg.buildcfg}/deps/%{prj.name}/int")

        files {
            "%{prj.location}/*.h",
            "%{prj.location}/*.cpp"
        }
    
        includedirs { "deps/dearimgui" }

        defines "IMGUI_IMPL_OPENGL_LOADER_GLAD"