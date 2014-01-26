
local base_dir = (solution().basedir .. "/external/polarssl/")

group "external"
    project "polarssl"
        kind "StaticLib"
        language "C"
        location (_ACTION .. "/" .. project().name)
        
        defines
        {
            "POLARSSL_EXPORTS"
        }
        includedirs
        {
            base_dir .. "include/"
        }
        files
        {
            base_dir .. "include/polarssl/*.h",
            base_dir .. "src/*.c"
        }
        links
        {
        }
    project "*"
group ""