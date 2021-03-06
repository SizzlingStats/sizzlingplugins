
local base_dir = (solution().basedir .. "/sizzlingrecord/")
local sizzlib_dir = (solution().basedir .. "/sizzlinglib/")

project "sizzlingrecord"
    kind "SharedLib"
    language "C++"
    location (_ACTION .. "/" .. project().name)
    
    defines
    {
    }
    includedirs
    {
        sizzlib_dir .. "include",
    }
    files
    {
        base_dir .. "**.h",
        base_dir .. "**.cpp",
        base_dir .. "../sizzlingplugins.licenseheader"
    }
    libdirs
    {
    }
    links
    {
        "SizzlingLib"
    }
    
    assert(loadfile("source-sdk-2013.lua"))(project().name)
project "*"
