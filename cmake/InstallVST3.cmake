function(install_vst3_bundle TARGET_NAME)
    if(NOT DEFINED VST3_INSTALL_DIR)
        if(WIN32)
            set(VST3_INSTALL_DIR "$ENV{LOCALAPPDATA}/Programs/Common/VST3/${TARGET_NAME}")
        elseif(UNIX AND NOT APPLE)
            set(VST3_INSTALL_DIR "$ENV{HOME}/.vst3/${TARGET_NAME}")
        endif()
    endif()

    if(NOT IS_ABSOLUTE "${VST3_INSTALL_DIR}")
        message(FATAL_ERROR "VST3_INSTALL_DIR must be an absolute path. Got: ${VST3_INSTALL_DIR}")
    endif()

    if(WIN32)
        set(VST3_SOURCE "${CMAKE_BINARY_DIR}/${TARGET_NAME}_artefacts/$<CONFIG>/VST3/${TARGET_NAME}VST3.vst3/Contents/x86_64-win/${TARGET_NAME}VST3.vst3")
        set(VST3_DEST "${VST3_INSTALL_DIR}/${TARGET_NAME}VST3.vst3")
    elseif(UNIX AND NOT APPLE)
        set(VST3_SOURCE "${CMAKE_BINARY_DIR}/${TARGET_NAME}_artefacts/VST3/${TARGET_NAME}VST3.vst3")
        set(VST3_DEST "${VST3_INSTALL_DIR}/")
    else()
        return()
    endif()

    add_custom_target(copy_vst3 ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory "${VST3_INSTALL_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${VST3_SOURCE}" "${VST3_DEST}"
        COMMENT "Copying VST3 to ${VST3_DEST}"
    )

    add_dependencies(copy_vst3 ${TARGET_NAME})
endfunction()
