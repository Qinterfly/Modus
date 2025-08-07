
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)
find_package(Eigen3 CONFIG REQUIRED)
find_package(Ceres CONFIG REQUIRED)

set(BACKEND_HEADERS
    fileutility.h
    selector.h
    selectionset.h
)

set(BACKEND_SOURCES
    selector.cpp
    selectionset.cpp
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
)
