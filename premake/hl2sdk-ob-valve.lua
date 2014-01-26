
-- this file is expected to be included from a project scope

local sizzplugins_dir = (solution().basedir .. "/")
local hl2sdk_dir = sizzplugins_dir .. "../hl2sdk-ob-valve/"

includedirs
{
    hl2sdk_dir .. "common/",
    hl2sdk_dir .. "public/",
    hl2sdk_dir .. "public/tier0/",
    hl2sdk_dir .. "public/tier1/",
    hl2sdk_dir .. "game/server/",
    hl2sdk_dir .. "game/shared/",
    
    -- not part of the sdk, but related to the game... kind of
    sizzplugins_dir .. "common"
}

configuration "windows"
    libdirs
    {
        hl2sdk_dir .. "lib/public/"
    }
    links
    {
        "mathlib",
        "tier0",
        "tier1",
        "tier2",
        "vstdlib"
    }
configuration "linux"
    defines
    {
        "stricmp=strcasecmp",
        "_stricmp=strcasecmp",
        "_strnicmp=strncasecmp",
        "strnicmp=strncasecmp",
        "_snprintf=snprintf",
        "_vsnprintf=vsnprintf",
        "_alloca=alloca",
        "strcmpi=strcasecmp"
    }
    libdirs
    {
        sizzplugins_dir .. "lib/linux"
    }
    links
    {
        "tier0_srv",
        "vstdlib_srv"
    }
    linkoptions
    {
        "-L" .. hl2sdk_dir .. "mathlib_i486.a",
        "-L" .. hl2sdk_dir .. "tier1_i486.a"
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
