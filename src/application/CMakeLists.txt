
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(application
    main.cpp
    ${RESOURCES_FILE}
    ${INFO_FILE}
)

set_target_properties(application PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME ${APP_NAME}
)

target_link_libraries(application PRIVATE
    Qt::Widgets
    backend
    frontend
)

target_include_directories(application PUBLIC ${CMAKE_BINARY_DIR})

install(TARGETS application
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt.debug.bat)
    else()
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
    endif()
    string(TOLOWER ${CMAKE_SYSTEM_NAME} systemName)
    string(TOLOWER ${CMAKE_BUILD_TYPE} configName)
    set(scriptDir ${PROJECT_SOURCE_DIR}/cmake)
    set(installDir ${ROOT_INSTALL_DIR}/${systemName}-${configName})
    set(targetFiles ${CMAKE_CURRENT_BINARY_DIR}/*)
    set(kclFiles ${KCL_DIR}/*)
    add_custom_command(TARGET application POST_BUILD
        COMMAND ${TOOL_WINDEPLOYQT} $<TARGET_FILE:application>
        COMMAND ${CMAKE_COMMAND} -E make_directory ${installDir}
        COMMAND ${CMAKE_COMMAND} "-DSRCS=${targetFiles}" "-DDEST=${installDir}" -P "${scriptDir}/copy.cmake"
        COMMAND ${CMAKE_COMMAND} "-DSRCS=${kclFiles}" "-DDEST=${installDir}" -P "${scriptDir}/copy.cmake"
        COMMAND ${CMAKE_COMMAND} "-DSRCS=${kclFiles}" "-DDEST=${CMAKE_CURRENT_BINARY_DIR}" -P "${scriptDir}/copy.cmake"
        COMMAND ${CMAKE_COMMAND} "-DSRCS=${EXAMPLES_DIR}" "-DDEST=${installDir}" -P "${scriptDir}/copy.cmake"
    )
endif()

