
local base_dir = (solution().basedir .. "/external/miniz_v115_r4/")

group "external"
    project "miniz"
        kind "StaticLib"
        language "C"
        location (_ACTION .. "/" .. project().name)
        
        defines
        {
        }
        includedirs
        {
            base_dir
        }
        files
        {
            base_dir .. "*.h",
            base_dir .. "*.c"
        }
        links
        {
        }
    project "*"
group ""
