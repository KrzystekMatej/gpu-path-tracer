add_executable(app)

target_sources(app PRIVATE
    "Source/Main.cpp"
    
  "Source/SceneViewerApp.cpp")

target_include_directories(app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Include")
target_link_libraries(app PRIVATE core)

foreach(dep_name IN LISTS APP_DEPENDENCIES)
    if(DEP_${dep_name}_TARGET AND TARGET ${DEP_${dep_name}_TARGET})
        target_link_libraries(app
            PRIVATE
                ${DEP_${dep_name}_TARGET}
        )
    elseif(DEP_${dep_name}_INCLUDE_DIR)
        target_include_directories(app
            PRIVATE
                ${DEP_${dep_name}_INCLUDE_DIR}
        )
    endif()
endforeach()
