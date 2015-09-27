
local base_dir = (solution().basedir .. "/sizzlingstats/")
local external_dir = (solution().basedir .. "/external/")
local sizzlib_dir = (solution().basedir .. "/sizzlinglib/")

local srcds_dir = "D:/Program Files/Valve/srcds/orangebox/"
local move_target_cmd = "move /Y \"$(TargetDir)/$(TargetName)$(TargetExt)\" "

project "sizzlingstats"
    kind "SharedLib"
    language "C++"
    location (_ACTION .. "/" .. project().name)
    
    debugcommand (srcds_dir .. "srcds.exe")
    debugargs "-insecure -console -game tf +map cp_badlands"
    
    configuration "vs*"
        postbuildcommands
        {
            move_target_cmd .. "\"" .. srcds_dir .. "tf/addons/sizzlingplugins/sizzlingstats/\""
        }
    configuration {}
    
    defines
    {
        "CURL_STATICLIB"
    }
    includedirs
    {
        sizzlib_dir .. "include",
        external_dir .. "libcurl/include",
        -- for protobuf
        external_dir
    }
    files
    {
        base_dir .. "**.h",
        base_dir .. "**.cpp",
        base_dir .. "**.pb.cc",
        base_dir .. "**.pb.h",
        base_dir .. "**.proto",
        base_dir .. "../sizzlingplugins.licenseheader"
    }
    libdirs
    {
    }
    links
    {
        "SizzlingLib",
        "libcurl",
        "polarssl",
        "protobuf_lite",
        "miniz"
    }
    
    dofile "source-sdk-2013.lua"
project "*"
