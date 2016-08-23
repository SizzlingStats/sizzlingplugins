
newoption
{
    trigger     = "srcds-path",
    value       = "path",
    description = "Path to your srcds installation. Used for debugging and binary deployment."
}

solution "SizzlingPlugins"
    basedir ".."
    location (_ACTION)
    startproject "sizzlingstats"
    platforms "x32"
    
    vectorextensions "SSE2"
    floatingpoint "Fast"
    warnings "Default"
    
    vpaths
    {
        ["Header Files"] = { "../**.h", "../**.hxx", "../**.hpp" },
        ["Source Files"] = { "../**.c", "../**.cxx", "../**.cpp", "../**.cc" },
        ["Protobuf"] = { "../**.proto" },
        ["*"] = "../**.licenseheader"
    }
    flags
    {
        "StaticRuntime",
        "NoExceptions"
    }
    defines
    {
    }
    
    -- build configurations
    configurations { "Debug", "Release" }
    configuration "Debug"
        targetdir (_ACTION .. "/build/Debug")
        defines
        {
            "_DEBUG"
        }
        flags
        {
            "Symbols"
        }
    configuration "Release"
        targetdir (_ACTION .. "/build/Release")
        optimize "Full"
        defines
        {
            "NDEBUG"
        }
        flags
        {
            "NoBufferSecurityCheck",
            "NoEditAndContinue",
            "NoIncrementalLink",
            --"LinkTimeOptimization",
            "MultiProcessorCompile",
            "NoRTTI"
        }
    
    -- os specific configurations
    configuration "windows"
        defines
        {
            "_WINDOWS",
            "_WIN32",
            "WIN32",
            "_CRT_SECURE_NO_DEPRECATE",
            "_CRT_NONSTDC_NO_DEPRECATE",
            "_CRT_SECURE_NO_WARNINGS",
            "_CRT_NONSTDC_NO_WARNINGS"
        }
    configuration "linux"
        defines
        {
            "_LINUX"
        }
        toolset "clang"
        targetprefix ""
    configuration { "linux", "C++" }
        buildoptions {
            "-std=c++11",
            "-isystem /usr/include/i386-linux-gnu/c++/4.8*",
            "-isystem /usr/include/i386-linux-gnu/c++/4.7*",
            "-isystem /usr/include/i386-linux-gnu/c++/4.6*"
        }
    
    -- compiler specific configurations
    configuration "vs*"
        buildoptions
        {
            -- warning C4530: C++ exception handler used,
            -- but unwind semantics are not enabled.
            "/EHsc"
        }
    configuration {}

    -- the projects
    dofile "libcurl.lua"
    dofile "polarssl.lua"
    dofile "snappy.lua"
    dofile "protobuf_lite.lua"
    dofile "miniz.lua"
    
    dofile "sizzlinglib.lua"
    dofile "sizzlingstats.lua"
    dofile "autotimer.lua"
    dofile "new_plugin.lua"
    dofile "sizzlingmatches.lua"
    dofile "sizzlingrecord.lua"
