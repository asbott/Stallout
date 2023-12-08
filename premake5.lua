
local modules = {}

local file = io.open("module_list", "r")

for line in file:lines() do
    table.insert(modules, line)
end


file:close() -- Close the file


function stallout_project(name) 
    project (name)
    warnings   "Extra"
    language   "C++"
    cppdialect "C++20"
    targetdir  ("bin/%{cfg.buildcfg}/%{wks.name}/%{prj.name}")
    objdir     ("bin/%{cfg.buildcfg}/%{wks.name}/%{prj.name}/int")
    vectorextensions "SSE4.1"
    location "%{prj.name}"
    includedirs {
        "%{prj.location}/include",
        "deps/mz",
        "deps/imgui",
        "deps/spdlog/include"
        
    }
    undefines { "UNICODE", "_UNICODE" }
    files { "%{prj.location}/src/**.cpp", "%{prj.location}/include/**.h" }
    flags {
        "FatalWarnings",
        "MultiProcessorCompile"
    }

    pchheader "pch.h"
    pchsource "%{prj.location}/src/pch.cpp"
end

function module_project(name)
    stallout_project(name)
    kind "SharedLib"

    includedirs {
        "Stallout/include",
        "deps/dearimgui",
        name .. "/include"
    }

    links {
        "Stallout",
        "dearimgui"
    }

    defines {
        "ST_MODULE_NAME=\"" .. name .. "\""
    }

    postbuildcommands {
        "{COPY} %{cfg.buildtarget.relpath} %{wks.location}bin/%{cfg.buildcfg}/%{wks.name}/Launcher/"
    }
end



workspace "Stallout"  

    configurations { "Debug", "Test", "Release" } 

    architecture "x64"
    systemversion "latest"

    startproject "Launcher"

    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "_CRTLDBG_REPORT_FLAG",
        "FMT_HEADER_ONLY",
        "IMGUI_IMPL_OPENGL_LOADER_XXX"
    }

    files { "*.natvis", "*premake5.lua", "module_list", "*.bat", ".gitignore", "_todo.txt", "scripts/**" }
    
    filter "platforms:*-opengl45*"
        defines { "_ST_RENDER_BACKEND_OPENGL", "_ST_RENDER_BACKEND_OPENGL45" }
    filter "platforms:*-vulkan*"
        defines { "_ST_RENDER_BACKEND_VULKAN" }
    filter "platforms:*-dx11*"
        defines { "_ST_RENDER_BACKEND_DX11" }
    filter "platforms:*-dx12*"
        defines { "_ST_RENDER_BACKEND_DX12" }
    filter "platforms:*-runtests*"
        defines { "_ST_RUN_TESTS", "ST_ENABLE_MEMORY_TRACKING" }

    filter "configurations:Debug*"
        defines {
            "_ST_CONFIG_DEBUG", 
            "_ST_ENABLE_GL_DEBUG_CONTEXT",
            "ST_ENABLE_MEMORY_META_DEBUG",
            "DEBUG",
            "_DEBUG"
        }
        runtime "Debug"
        symbols "On"
        floatingpoint "Default"

    filter "configurations:Test*"
        defines  {
            "_ST_CONFIG_TEST"
        }
        runtime "Release"
        symbols "On"
        optimize "Speed"
        floatingpoint "Fast"

    filter "configurations:Release*"
        defines  { 
            "_ST_CONFIG_RELEASE", 
            "_ST_DISABLE_ASSERTS", 
            "NDEBUG" 
        }
        runtime  "Release"
        symbols  "Off"
        optimize "Speed"
        floatingpoint "Fast"

    

    filter "system:windows"
        platforms { "x64-opengl45", "x64-opengl45-runtests", "x64-dx11", "x64-dx11-runtests", "x64-dx12", "x64-dx12-runtests", "x64-vulkan", "x64-vulkan-runtests" }
        defines { "_ST_OS_WINDOWS" }
        exceptionhandling "SEH"

    filter "system:linux"
        platforms { "x64-opengl45", "x64-opengl45-runtests", "x64-vulkan", "x64-vulkan-runtests" }
        pic "On"
        defines { "_ST_OS_LINUX" }

    filter "action:vs*"
        buildoptions { "/wd4201", "/wd26495" }

    filter ""
    group "modules"
        function write_modules_file(cfg)
            local file = io.open("bin/".. cfg .. "/Stallout/Launcher/module_list", "w+")
            -- Write the module names into the file
            local first = true
            for _, moduleName in ipairs(modules) do
                if moduleName ~= "" then
                    if first then
                        file:write(moduleName)
                        first = false
                    else
                        file:write("\n" .. moduleName)
                    end
                    module_project(moduleName);
                end
            end
            file:close()
        end
        write_modules_file("Debug");
        write_modules_file("Test");
        write_modules_file("Release");    

    group "core"

    stallout_project "Launcher"
        kind "ConsoleApp"

        includedirs {
            "Stallout/include"
        }

        links {
            "Stallout"
        }

        defines {
            "ST_MODULE_NAME=\"Launcher\""
        }

        postbuildcommands {
            "{COPY} %{wks.location}deps/openal/lib/%{cfg.buildcfg}/OpenAL32.dll %{wks.location}bin/%{cfg.buildcfg}/%{wks.name}/Launcher/"
        }



    stallout_project "Stallout"
        kind "SharedLib"
        
        postbuildcommands {
            "{COPY} %{cfg.buildtarget.relpath} %{wks.location}bin/%{cfg.buildcfg}/%{wks.name}/Launcher//"
        }

        defines {
            "GLFW_STATIC",
            "GLAD_STATIC",
            "_ST_EXPORT",
            "ST_CORE",
            "ST_MODULE_NAME=\"Core\"",
            --"FT2_BUILD_LIBRARY"
        }

        includedirs {
            "Stallout/include/Stallout",
            "deps/glad/include",
            "deps/dearimgui",
            "deps/glfw/include",
            "deps/miniaudio",
            "deps/stb",
            "deps/openal/repo/include",
            "deps/freetype2/include"
        }

        links {
            "dearimgui",
            "glfw",
            "glad",
            "OpenAL32",
            "freetype2"
        }

        removefiles {
            "%{prj.location}/include/os/**",
            "%{prj.location}/src/os/**",
            "%{prj.location}/src/graphics/**"
        }

        files {
            "%{prj.location}/include/os/*.h",
            "%{prj.location}/src/os/*.cpp",
            "%{prj.location}/src/graphics/*"
        }

        filter "platforms:*-opengl45*"
            files {
                "%{prj.location}/src/graphics/opengl45/**"
            }
        filter "platforms:*-vulkan*"
            files {
                "%{prj.location}/src/graphics/vulkan/**"
            }
        filter "platforms:*-dx11*"
            files {
                "%{prj.location}/src/graphics/dx11/**"
            }
        filter "platforms:*-dx12*"
            files {
                "%{prj.location}/src/graphics/dx12/**"
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

            filter "platforms:*-opengl*"
                files {
                    "%{prj.location}/src/os/windows/opengl/**"
                }
            filter "platforms:*-vulkan*"
                files {
                    "%{prj.location}/src/os/windows/vulkan/**"
                }
            filter "platforms:*-dx11*"
                files {
                    "%{prj.location}/src/os/windows/dx11/**"
                }
            filter "platforms:*-dx12*"
                files {
                    "%{prj.location}/src/os/windows/dx12/**"
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

            files {
                "%{prj.location}/include/os/linux/*.h",
                "%{prj.location}/src/os/linux/*.cpp",
            }

            filter "platforms:*-opengl*"
                files {
                    "%{prj.location}/src/os/linux/opengl/**"
                }
            filter "platforms:*-vulkan*"
                files {
                    "%{prj.location}/src/os/linux/vulkan/**"
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
        

    group "deps"

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
    
        includedirs { 
            "deps/dearimgui"
        }

        filter "platforms:*-opengl45*"
            files {
                "%{prj.location}/backends/imgui_impl_opengl3*"
            }
        filter "platforms:*-vulkan*"
            files {
                "%{prj.location}/backends/imgui_impl_vulkan*"
            }
        filter "platforms:*-dx11*"
            files {
                "%{prj.location}/backends/imgui_impl_dx11*"
            }
        filter "platforms:*-dx12*"
            files {
                "%{prj.location}/backends/imgui_impl_dx12*"
            }

        filter "system:windows"
            files {
                "%{prj.location}/backends/imgui_impl_win32*"
            }

        filter "system:linux"
            pic "On"
            files {
                "%{prj.location}/backends/imgui_impl_glfw*"
            }

    project "freetype2"
        location   "deps/freetype2"
        kind       "StaticLib"
        language   "C++"
        cppdialect "C++20"
        warnings   "Off"
        targetdir  ("bin/%{cfg.buildcfg}/deps/%{prj.name}")
        objdir     ("bin/%{cfg.buildcfg}/deps/%{prj.name}/int")

        defines {
            "FT2_BUILD_LIBRARY"
        }

        files {
            "%{prj.location}/include/**",
            "%{prj.location}/src/base/ftbbox.c",
            "%{prj.location}/src/base/ftbdf.c",
            "%{prj.location}/src/base/ftbitmap.c",
            "%{prj.location}/src/base/ftcid.c",
            "%{prj.location}/src/base/ftfstype.c",
            "%{prj.location}/src/base/ftgasp.c",
            "%{prj.location}/src/base/ftglyph.c",
            "%{prj.location}/src/base/ftgxval.c",
            "%{prj.location}/src/base/ftmm.c",
            "%{prj.location}/src/base/ftotval.c",
            "%{prj.location}/src/base/ftpatent.c",
            "%{prj.location}/src/base/ftpfr.c",
            "%{prj.location}/src/base/ftstroke.c",
            "%{prj.location}/src/base/ftsynth.c",
            "%{prj.location}/src/base/fttype1.c",
            "%{prj.location}/src/base/ftwinfnt.c",
            "%{prj.location}/src/base/ftinit.c",
            "%{prj.location}/src/base/ftbase.c",


            "%{prj.location}/src/autofit/autofit.c",
            "%{prj.location}/src/truetype/truetype.c",
            "%{prj.location}/src/type1/type1.c",
            "%{prj.location}/src/cff/cff.c",
            "%{prj.location}/src/cid/type1cid.c",
            "%{prj.location}/src/pfr/pfr.c",
            "%{prj.location}/src/type42/type42.c",
            "%{prj.location}/src/winfonts/winfnt.c",
            "%{prj.location}/src/gzip/ftgzip.c",
            "%{prj.location}/src/bdf/bdf.c",
            "%{prj.location}/src/psaux/psaux.c",
            "%{prj.location}/src/psnames/psnames.c",
            "%{prj.location}/src/pshinter/pshinter.c",
            "%{prj.location}/src/sfnt/sfnt.c",
            "%{prj.location}/src/smooth/smooth.c",
            "%{prj.location}/src/raster/raster.c",
            "%{prj.location}/src/sdf/sdf.c",
            "%{prj.location}/src/svg/svg.c",

        }

        removefiles {
            "%{prj.location}/src/gzip/inffast.c",
            "%{prj.location}/src/gzip/inflate.c",
            "%{prj.location}/src/tools/ftrandom/ftrandom.c"
        }

        includedirs { 
            "%{prj.location}/include",
            "%{prj.location}/include/freetype",
            "%{prj.location}/include/dlg"
        }
        filter "system:windows"
            files {
                "%{prj.location}/builds/windows/*.c"
            }

        filter "system:linux"
            pic "On"
            files {
                "%{prj.location}/builds/unix/*.c"
            }