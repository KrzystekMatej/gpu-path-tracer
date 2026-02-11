include(FetchContent)

function(is_resolved OUT_VAR)
    if(TARGET_NAME)
        if(TARGET ${TARGET_NAME})
            set(${OUT_VAR} TRUE PARENT_SCOPE)
            return()
        endif()
    endif()
    if(INCLUDE_DIR)
        if(EXISTS "${INCLUDE_DIR}")
            set(${OUT_VAR} TRUE PARENT_SCOPE)
            return()
        endif()
    endif()
    if(NOT TARGET_NAME AND NOT INCLUDE_DIR)
        message(FATAL_ERROR
            "Dependency definition '${DEP_NAME}' must provide either TARGET or INCLUDE_DIR"
        )
    endif()

	set(${OUT_VAR} FALSE PARENT_SCOPE)
endfunction()

function(resolve_dependency DEP_NAME)
    set(NAME                ${DEP_${DEP_NAME}_NAME})
    set(FIND_PACKAGE_NAME   ${DEP_${DEP_NAME}_FIND_PACKAGE_NAME})
    set(GIT_REPOSITORY      ${DEP_${DEP_NAME}_GIT_REPOSITORY})
    set(GIT_TAG             ${DEP_${DEP_NAME}_GIT_TAG})
    set(TARGET_NAME       ${DEP_${DEP_NAME}_TARGET})
    set(PKG_CONFIG_NAME   ${DEP_${DEP_NAME}_PKG_CONFIG_NAME})
    set(INCLUDE_DIR ${DEP_${DEP_NAME}_INCLUDE_DIR})
    set(FIND_PATH_HEADER      ${DEP_${DEP_NAME}_FIND_PATH_HEADER})
    set(FIND_LIBRARY_NAME     ${DEP_${DEP_NAME}_FIND_LIBRARY_NAME})

    if(NOT NAME OR NOT FIND_PACKAGE_NAME)
        message(FATAL_ERROR "Dependency spec '${DEP_NAME}' is incomplete")
    endif()

    is_resolved(ok)
    if(ok)
        return()
    endif()

    set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/External/${NAME}")

	if(EXISTS "${EXTERNAL_DIR}")
		if(EXISTS "${EXTERNAL_DIR}/CMakeLists.txt")
			add_subdirectory(
				"${EXTERNAL_DIR}"
				"${CMAKE_BINARY_DIR}/_deps/${NAME}-build"
			)
        endif()
        
        is_resolved(ok)
        if(ok)
            return()
        endif()

        message(FATAL_ERROR
            "External/${NAME} was added but did not provide required TARGET/INCLUDE_DIR."
        )
	endif()

    find_package(${FIND_PACKAGE_NAME} CONFIG QUIET)
    is_resolved(ok)
    if(ok)
        return()
    endif()
    # for modules (e.g. opengl)
    find_package(${FIND_PACKAGE_NAME} QUIET)
    is_resolved(ok)
    if(ok)
        return()
    endif()
    
    if(UNIX AND TARGET_NAME AND PKG_CONFIG_NAME)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(PC_${DEP_NAME} QUIET ${PKG_CONFIG_NAME})
            if(PC_${DEP_NAME}_FOUND AND NOT TARGET ${TARGET_NAME})
                add_library(${TARGET_NAME} INTERFACE)
                target_include_directories(${TARGET_NAME} INTERFACE ${PC_${DEP_NAME}_INCLUDE_DIRS})
                target_link_libraries(${TARGET_NAME} INTERFACE ${PC_${DEP_NAME}_LIBRARIES})
            endif()
        endif()
    endif()
    
    is_resolved(ok)
    if(ok)
        return()
    endif()

    if(UNIX AND TARGET_NAME AND FIND_PATH_HEADER AND FIND_LIBRARY_NAME)
        find_path(${DEP_NAME}_INCLUDE_DIR ${FIND_PATH_HEADER})
        find_library(${DEP_NAME}_LIBRARY ${FIND_LIBRARY_NAME})

        if(${DEP_NAME}_INCLUDE_DIR AND ${DEP_NAME}_LIBRARY AND NOT TARGET ${TARGET_NAME})
            add_library(${TARGET_NAME} UNKNOWN IMPORTED)
            set_target_properties(${TARGET_NAME} PROPERTIES
                IMPORTED_LOCATION "${${DEP_NAME}_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${${DEP_NAME}_INCLUDE_DIR}"
            )
        endif()
    endif()
    
    is_resolved(ok)
    if(ok)
        return()
    endif()

    if(GIT_REPOSITORY AND GIT_TAG)
        FetchContent_Declare(
            ${NAME}
            GIT_REPOSITORY ${GIT_REPOSITORY}
            GIT_TAG        ${GIT_TAG}
            SOURCE_DIR     "${EXTERNAL_DIR}"
        )
        FetchContent_MakeAvailable(${NAME})

        is_resolved(ok)
        if(ok)
            return()
        endif()

        message(FATAL_ERROR
            "FetchContent for '${NAME}' completed but did not provide required target '${TARGET_NAME}'."
        )
    endif()

    message(FATAL_ERROR
        "Dependency '${NAME}' not resolved.\n"
    )
endfunction()
