
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(VTK REQUIRED COMPONENTS
    CommonColor
    CommonCore
    CommonDataModel
    FiltersHybrid
    FiltersSources
    FiltersGeometry
    InteractionStyle
    InteractionWidgets
    RenderingAnnotation
    RenderingContextOpenGL2
    RenderingCore
    RenderingFreeType
    RenderingGL2PSOpenGL2
    RenderingOpenGL2
    GUISupportQt
    RenderingQt
)

qt_add_executable(testfrontend
    testfrontend.h
    testfrontend.cpp
    ${RESOURCES_FILE}
)

set_target_properties(testfrontend PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(testfrontend PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Test
    ${VTK_LIBRARIES}
    backend
    frontend
)

target_include_directories(testfrontend PUBLIC ${CMAKE_BINARY_DIR})

vtk_module_autoinit(
    TARGETS testfrontend
    MODULES ${VTK_LIBRARIES}
)

if (WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt.debug.bat)
    else()
        find_program(TOOL_WINDEPLOYQT NAMES windeployqt)
    endif()
   set(scriptDir ${PROJECT_SOURCE_DIR}/cmake)
   set(kclFiles ${KCL_DIR}/*)
   add_custom_command(TARGET testfrontend POST_BUILD
       COMMAND ${TOOL_WINDEPLOYQT} $<TARGET_FILE:testfrontend>
       COMMAND ${CMAKE_COMMAND} "-DSRCS=${kclFiles}" "-DDEST=${CMAKE_CURRENT_BINARY_DIR}" -P "${scriptDir}/copy.cmake"
   )
endif()
