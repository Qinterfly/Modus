
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGLWidgets)
find_package(qtadvanceddocking-qt6 CONFIG REQUIRED)
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

set(FRONTEND_HEADERS
    mainwindow.h
    uialiasdata.h
    uiconstants.h
    uiutility.h
    logger.h
    projectbrowser.h
    projecthierarchymodel.h
    hierarchyitem.h
    iview.h
    viewmanager.h
    modelview.h
    editormanager.h
    lineedit.h
    beameditor.h
    generaldataeditor.h
    paneleditor.h
    masseditor.h
    constantseditor.h
    analysisparameterseditor.h
    aerotrapeziumeditor.h
    rawdataeditor.h
    customtable.h
    polyexponentseditor.h
    springdampereditor.h
    geometryview.h
    modeleditor.h
    modaloptionseditor.h
    flutteroptionseditor.h
    optimoptionseditor.cpp
)

set(FRONTEND_SOURCES
    mainwindow.cpp
    uiutility.cpp
    logger.cpp
    projectbrowser.cpp
    projecthierarchymodel.cpp
    hierarchyitem.cpp
    viewmanager.cpp
    modelview.cpp
    editormanager.cpp
    lineedit.cpp
    beameditor.cpp
    generaldataeditor.cpp
    paneleditor.cpp
    masseditor.cpp
    constantseditor.cpp
    analysisparameterseditor.cpp
    aerotrapeziumeditor.cpp
    rawdataeditor.cpp
    customtable.cpp
    polyexponentseditor.cpp
    springdampereditor.cpp
    geometryview.cpp
    modeleditor.cpp
    modaloptionseditor.cpp
    flutteroptionseditor.cpp
    optimoptionseditor.cpp
)

qt_add_library(frontend STATIC
    ${FRONTEND_HEADERS}
    ${FRONTEND_SOURCES}
    ${RESOURCES_FILE}
)

target_link_libraries(frontend PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::OpenGLWidgets
    ads::qtadvanceddocking-qt6
    ${VTK_LIBRARIES}
    qtpropertybrowser
    backend
)

target_include_directories(frontend PUBLIC ${CMAKE_BINARY_DIR})
target_include_directories(frontend INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

vtk_module_autoinit(
    TARGETS frontend
    MODULES ${VTK_LIBRARIES}
)

qt_add_translations(frontend
    TS_FILES ${TRANSLATIONS_DIR}/application_ru.ts ${TRANSLATIONS_DIR}/application_en.ts
    RESOURCE_PREFIX "/translations"
    -no-obsolete
)

