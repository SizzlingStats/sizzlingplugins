
local base_dir = (solution().basedir .. "/sizzlinglib/")
local external_dir = (solution().basedir .. "/external/")

project "SizzlingLib"
    kind "StaticLib"
    language "C++"
    location (_ACTION .. "/" .. project().name)
    
    defines
    {
        "CURL_STATICLIB",
        "SUPPRESS_INVALID_PARAMETER_NO_INFO"
    }
    includedirs
    {
        base_dir .. "include/",
        external_dir .. "libcurl/include"
    }
    files
    {
        base_dir .. "include/*.h",
        base_dir .. "*.cpp",
        external_dir .. "public/XZip.cpp",
        external_dir .. "../external/tier0/memoverride.cpp",
        base_dir .. "../sizzlingplugins.licenseheader"
    }
    links
    {
        "libcurl"
    }
    
    configuration "windows"
        linkoptions
        {
            "/NODEFAULTLIB"
        }
    configuration {}
    
    dofile "hl2sdk-ob-valve.lua"
