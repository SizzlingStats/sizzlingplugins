
local base_dir = (solution().basedir .. "/sizzlingstats/")
local external_dir = (solution().basedir .. "/external/")
local sizzlib_dir = (solution().basedir .. "/sizzlinglib/")

project "SizzlingStats"
    kind "SharedLib"
    language "C++"
    location (_ACTION .. "/" .. project().name)
    
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
        base_dir .. "**.proto"
    }
    libdirs
    {
    }
    links
    {
        "SizzlingLib",
        "libcurl",
        "protobuf_lite"
    }
    
    dofile "hl2sdk-ob-valve.lua"
