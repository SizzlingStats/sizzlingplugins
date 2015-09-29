
local base_dir = (solution().basedir .. "/sizzlinglib/")
local external_dir = (solution().basedir .. "/external/")

project "sizzlinglib"
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
        external_dir .. "libcurl/include",
        external_dir .. "miniz_v115_r4",
        external_dir .. "rapidjson-1.0.2/include"
    }
    files
    {
        base_dir .. "include/*.h",
        base_dir .. "*.cpp",
        base_dir .. "../sizzlingplugins.licenseheader"
    }
    links
    {
        "libcurl",
        "miniz"
    }
    
    configuration "windows"
        linkoptions
        {
            "/NODEFAULTLIB"
        }
    configuration {}
    
    assert(loadfile("source-sdk-2013.lua"))(project().name)
project "*"
