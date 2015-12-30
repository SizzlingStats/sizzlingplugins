
local base_dir = (solution().basedir .. "/sizzlingstats/")
local external_dir = (solution().basedir .. "/external/")
local sizzlib_dir = (solution().basedir .. "/sizzlinglib/")

local srcds_path = _OPTIONS["srcds-path"]
local srcds_exe = nil
local postbuild_move = nil
if srcds_path ~= nil then
    
    local move_target_cmd = "move /Y \"$(TargetDir)/$(TargetName)$(TargetExt)\" "
    srcds_exe = srcds_path .. "srcds.exe"
    postbuild_move = move_target_cmd .. "\"" .. srcds_path .. "tf/addons/sizzlingplugins/sizzlingstats/\""
end

project "sizzlingstats"
    kind "SharedLib"
    language "C++"
    location (_ACTION .. "/" .. project().name)
    
    debugcommand (srcds_exe)
    debugargs "-insecure -console -game tf +map cp_badlands"
    
    configuration "vs*"
        postbuildcommands
        {
            postbuild_move
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
    
    assert(loadfile("source-sdk-2013.lua"))(project().name)
project "*"
