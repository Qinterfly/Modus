
find_package(Qt6 REQUIRED COMPONENTS Core Test)
find_package(Eigen3 CONFIG REQUIRED)

qt_add_executable(testbackend
    testbackend.h
    testbackend.cpp
)

set_target_properties(testbackend PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(testbackend PRIVATE
    Qt::Core
    Qt::Test
    Eigen3::Eigen
    backend
)

target_include_directories(testbackend PUBLIC
    ${CMAKE_BINARY_DIR}
)
