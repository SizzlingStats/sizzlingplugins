
project "SizzlingLib"
    kind "StaticLib"
    language "C++"

    includedirs "include"
    files
    {
        "include/*.h",
        "*.cpp",
        "../external/public/XZip.cpp",
        "../external/tier0/memoverride.cpp",
        "../*.licenseheader"
    }
    