function(configure_build_types)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug
        Profile
        Release
        RelWithDebInfo
        MinSizeRel
    )

    set(CMAKE_MAP_IMPORTED_CONFIG_PROFILE
        Release
        RelWithDebInfo
        MinSizeRel
        ""
        CACHE STRING "Imported target configuration fallback for Profile builds"
        FORCE
    )

    foreach(language IN ITEMS C CXX CUDA)
        set(
            CMAKE_${language}_FLAGS_PROFILE
            "${CMAKE_${language}_FLAGS_RELEASE}"
            CACHE STRING
            "Flags for ${language} Profile builds"
            FORCE
        )
    endforeach()

    foreach(linker_type IN ITEMS EXE SHARED MODULE)
        set(
            CMAKE_${linker_type}_LINKER_FLAGS_PROFILE
            "${CMAKE_${linker_type}_LINKER_FLAGS_RELEASE}"
            CACHE STRING
            "Linker flags for ${linker_type} Profile builds"
            FORCE
        )
    endforeach()

    add_compile_definitions(
        "$<$<CONFIG:Debug>:CORE_DEBUG>"
        "$<$<CONFIG:Profile>:CORE_ENABLE_TIMING>"
    )
endfunction()

function(configure_compiler_options)
    if(MSVC)
        add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:/W4>")

        string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" PARENT_SCOPE)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" PARENT_SCOPE)

        add_compile_options("$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C,CXX>>:/fsanitize=address>")

        add_link_options("$<$<CONFIG:Debug>:/fsanitize=address>")
        add_link_options("$<$<CONFIG:Debug>:/INCREMENTAL:NO>")
    endif()
endfunction()

function(configure_build_policy)
    configure_build_types()
    configure_compiler_options()
endfunction()