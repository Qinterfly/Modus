
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
find_package(Eigen3 CONFIG REQUIRED)
find_package(Ceres CONFIG REQUIRED)

set(BACKEND_HEADERS
    aliasdata.h
    constants.h
    identifier.h
    iserializable.h
    isolver.h
    fileutility.h
    mathutility.h
    project.h
    subproject.h
    optimselector.h
    selectionset.h
    optimconstraints.h
    geometry.h
    modalsolver.h
    optimsolver.h
    fluttersolver.h
)

set(BACKEND_SOURCES
    identifier.cpp
    fileutility.cpp
    mathutility.cpp
    project.cpp
    subproject.cpp
    optimselector.cpp
    selectionset.cpp
    optimconstraints.cpp
    geometry.cpp
    modalsolver.cpp
    optimsolver.cpp
    fluttersolver.cpp
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
