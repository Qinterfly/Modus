
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
find_package(Eigen3 CONFIG REQUIRED)
find_package(Ceres CONFIG REQUIRED)

set(BACKEND_HEADERS
    aliasdata.h
    constants.h
    identifier.h
    iserializable.h
    fileutility.h
    mathutility.h
    project.h
    subproject.h
    selector.h
    selectionset.h
    constraints.h
    optimsolver.h
    modalsolution.h
)

set(BACKEND_SOURCES
    identifier.cpp

    fileutility.cpp
    mathutility.cpp
    project.cpp
    subproject.cpp
    selector.cpp
    selectionset.cpp
    constraints.cpp
    optimsolver.cpp
    modalsolution.cpp
)

qt_add_library(backend STATIC
    ${BACKEND_HEADERS}
    ${BACKEND_SOURCES}
)

target_include_directories(backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(backend PUBLIC
    Qt::Core
    Eigen3::Eigen
    Ceres::ceres
    ${KCL_LIB}
    magicenum
)
