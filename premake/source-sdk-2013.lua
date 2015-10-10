
local projname = ...

-- this file is expected to be included from a project scope

local sizzplugins_dir = (solution().basedir .. "/")
local hl2sdk_dir = sizzplugins_dir .. "../source-sdk-2013/mp/src/"
local sdklib_dir = hl2sdk_dir .. "lib/public/"
local sizzlib_dir = sizzplugins_dir .. "lib/"
local external_dir = (solution().basedir .. "/external/")

project(projname)
    includedirs
    {
        -- not part of the sdk, but related to the game... kind of.
        -- These paths should be resolved first in case
        -- any files from the sdk need to be overridden.
        sizzplugins_dir .. "common/",
        external_dir .. "public/",
    
        hl2sdk_dir .. "common/",
        hl2sdk_dir .. "public/",
        hl2sdk_dir .. "public/tier0/",
        hl2sdk_dir .. "public/tier1/",
        hl2sdk_dir .. "game/server/",
        hl2sdk_dir .. "game/shared/"
    }
    links
    {
        "mathlib",
        "tier0",
        "tier1",
        "tier2",
        "vstdlib"
    }
    defines
    {
        "RAD_TELEMETRY_DISABLED",
        "MOVE_CONSTRUCTOR_SUPPORT"
    }

    configuration "windows"
        libdirs
        {
            sdklib_dir
        }
        files
        {
            hl2sdk_dir .. "public/tier0/memoverride.cpp"
        }
    configuration "linux"
        prelinkcommands
        {
            "ln -sf " .. (sdklib_dir .. "linux32/mathlib.a") .. " " .. (sizzlib_dir .. "libmathlib.a"),
            "ln -sf " .. (sdklib_dir .. "linux32/tier1.a") .. " " .. (sizzlib_dir .. "libtier1.a"),
            "ln -sf " .. (sdklib_dir .. "linux32/tier2.a") .. " " .. (sizzlib_dir .. "libtier2.a")
        }
        defines
        {
            "GNUC",
            "POSIX",
            "_LINUX",
            "LINUX",
            "NO_MALLOC_OVERRIDE"
        }
        libdirs
        {
            sdklib_dir .. "linux32/",
            sizzlib_dir
        }
        linkoptions
        {
            --"-Wl,--verbose"
        }
    configuration { "windows" }
        linkoptions
        {
            "/NODEFAULTLIB:libc.lib",
            "/NODEFAULTLIB:msvcrt.lib",
            "/NODEFAULTLIB:libcd.lib",
            "/NODEFAULTLIB:msvcrtd.lib"
        }
    configuration { "windows", "Debug" }
        linkoptions
        {
            "/NODEFAULTLIB:libcmt.lib",
        }
    configuration { "windows", "Release" }
        linkoptions
        {
            "/NODEFAULTLIB:libcmtd.lib"
        }
    configuration {}
