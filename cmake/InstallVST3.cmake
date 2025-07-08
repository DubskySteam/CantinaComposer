function(install_plug TARGET_NAME)
    if(WIN32)
        set(VST3_DEST_DIR "$ENV{LOCALAPPDATA}/Programs/Common/VST3")
        set(VST3_SOURCE_PATH "${CMAKE_BINARY_DIR}/${TARGET_NAME}_artefacts/Release/VST3/${TARGET_NAME}VST3.vst3")
    else()
        set(VST3_DEST_DIR "$ENV{HOME}/.vst3")
        set(VST3_SOURCE_PATH "${CMAKE_BINARY_DIR}/${TARGET_NAME}_artefacts/VST3/${TARGET_NAME}VST3.vst3")
    endif()

    add_custom_target(CopyVST3 ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${VST3_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VST3_SOURCE_PATH}"
            "${VST3_DEST_DIR}/${TARGET_NAME}VST3.vst3"
        DEPENDS ${TARGET_NAME}
        COMMENT "Copying VST3 plugin to ${VST3_DEST_DIR}"
        VERBATIM
    )
    
    add_dependencies(CopyVST3 ${TARGET_NAME})
endfunction()