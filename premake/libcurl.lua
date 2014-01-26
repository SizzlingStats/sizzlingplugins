
local base_dir = (solution().basedir .. "/external/libcurl/")

group "external"
    project "libcurl"
        kind "StaticLib"
        language "C"
        location (_ACTION .. "/" .. project().name)
        
        defines
        {
            "HTTP_ONLY",
            "USE_POLARSSL",
            "CURL_STATICLIB"
        }
        includedirs
        {
            base_dir .. "include/",
            base_dir .. "../polarssl/include"
        }
        files
        {
            base_dir .. "src/*.h",
            base_dir .. "src/*.c"
        }
        links
        {
            "polarssl"
        }
        configuration "linux"
            defines
            {
                "HAVE_CONFIG_H"
            }
        configuration {}
    project "*"
group ""
