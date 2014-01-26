
local external_dir = (solution().basedir .. "/external/")
local base_dir = (external_dir .. "google/protobuf/")

group "external"
    project "protobuf_lite"
        kind "StaticLib"
        language "C++"
        location (_ACTION .. "/" .. project().name)
        
        defines
        {
        }
        includedirs
        {
            external_dir,
            base_dir,
            base_dir .. "io",
            base_dir .. "stubs"
        }
        files
        {
            base_dir .. "io/coded_stream.h",
            base_dir .. "io/coded_stream_inl.h",
            base_dir .. "io/zero_copy_stream.h",
            base_dir .. "io/zero_copy_stream_impl_lite.h",
            
            base_dir .. "io/coded_stream.cc",
            base_dir .. "io/zero_copy_stream.cc",
            base_dir .. "io/zero_copy_stream_impl_lite.cc",
            
            base_dir .. "stubs/common.h",
            base_dir .. "stubs/hash.h",
            base_dir .. "stubs/map-util.h",
            base_dir .. "stubs/once.h",
            base_dir .. "stubs/atomicops.h",
            base_dir .. "stubs/atomicops_internals_atomicword_compat.h",
            base_dir .. "stubs/atomicops_internals_x86_gcc.h",
            base_dir .. "stubs/atomicops_internals_x86_msvc.h",
            base_dir .. "stubs/platform_macros.h",
            base_dir .. "stubs/stl_util.h",
            base_dir .. "stubs/stringprintf.h",
            base_dir .. "stubs/template_util.h",
            base_dir .. "stubs/type_traits.h",
            
            base_dir .. "stubs/common.cc",
            base_dir .. "stubs/once.cc",
            base_dir .. "stubs/atomicops_internals_x86_gcc.cc",
            base_dir .. "stubs/atomicops_internals_x86_msvc.cc",
            base_dir .. "stubs/stringprintf.cc",
            
            base_dir .. "extension_set.h",
            base_dir .. "generated_message_util.h",
            base_dir .. "message_lite.h",
            base_dir .. "repeated_field.h",
            base_dir .. "wire_format_lite.h",
            base_dir .. "wire_format_lite_inl.h",
            
            base_dir .. "extension_set.cc",
            base_dir .. "generated_message_util.cc",
            base_dir .. "message_lite.cc",
            base_dir .. "repeated_field.cc",
            base_dir .. "wire_format_lite.cc",
        }
        links
        {
        }

        configuration "vs*"
            buildoptions
            {
                "/wd4244 /wd4267 /wd4018 /wd4355 /wd4800 /wd4251 /wd4996 /wd4146 /wd4305"
            }
    project "*"
group ""
