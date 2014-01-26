
local base_dir = (solution().basedir .. "/external/google/snappy/")

group "external"
    project "snappy"
        kind "StaticLib"
        language "C++"
        location (_ACTION .. "/" .. project().name)
        
        defines
        {
        }
        includedirs
        {
            base_dir .. "include/"
        }
        files
        {
            base_dir .. "include/*.h",
            base_dir .. "include/*.h.in",
            base_dir .. "*.cc"
        }
        excludes
        {
            base_dir .. "**test.cc",
            base_dir .. "**test.h"
        }
        links
        {
        }
    project "*"
group ""
