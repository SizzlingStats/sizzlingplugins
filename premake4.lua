

solution "SizzlingPlugins"
    configurations { "Debug", "Release" }
    location "build"
    includedirs
    {
        "../hl2sdk-ob-valve/common",
        "../hl2sdk-ob-valve/public",
        "../hl2sdk-ob-valve/public/tier0",
        "../hl2sdk-ob-valve/public/tier1",
        "../hl2sdk-ob-valve/game/server",
        "../hl2sdk-ob-valve/game/shared",
        
        "common",
        "external/google/protobuf",
        "external/google/snappy",
        "external/libcurl/include",
        "external/open_steamworks",
        "external/polarssl/include"
    }
    vpaths
    {
        ["Header Files"] = "**.h",
        ["Source Files"] = "**.cpp",
        ["*"] = "*.licenseheader"
    }
    
    flags
    {
        "NoExceptions",
        "NoRTTI",
        "StaticRuntime"
    }

    configuration "Debug"
        targetdir "build/Debug"
        defines { "DEBUG", "_DEBUG" }
        flags
        {
            "Symbols"
        }

    configuration "Release"
        targetdir "build/Release"
        defines { "NDEBUG" }
        flags
        {
            "EnableSSE",
            "EnableSSE2",
            "ExtraWarnings",
            "FatalWarnings",
            "FloatFast",
            "LinkTimeOptimization",
            "NoEditAndContinue",
            "NoIncrementalLink",
            "NoMinimalRebuild",
            "Optimize"
        }

    include "SizzlingLib"
