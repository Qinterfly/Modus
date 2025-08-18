
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
find_package(Eigen3 CONFIG REQUIRED)
find_package(Ceres CONFIG REQUIRED)

set(BACKEND_HEADERS
    identifier.h
    fileutility.h
    mathutility.h
    project.h
    abstractsubproject.h
    kclsubproject.h
    selector.h
    selectionset.h
    constraints.h
    optimsolver.h
)

set(BACKEND_SOURCES
    identifier.cpp
    mathutility.cpp
    project.cpp
    abstractsubproject.cpp
    kclsubproject.cpp
    selector.cpp
    selectionset.cpp
    constraints.cpp
    optimsolver.cpp
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
