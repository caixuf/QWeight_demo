cmake_minimum_required(VERSION 3.16)

project(QWeight_demo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 解决MinGW编译器问题
if(MINGW)
    add_compile_definitions(_WIN32_WINNT=0x0601)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Sql
    Xml
    Concurrent
)

# Set Qt6 to be used
qt6_standard_project_setup()

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/GridGraphicsView.cpp
    src/ControlPanel.cpp
    src/ResultListWidget.cpp
    src/PathCalculator.cpp
    src/DataManager.cpp
    src/HistoryManager.cpp
    src/GridPoint.cpp
    src/PathResult.cpp
    src/Common.cpp
)

# Header files
set(HEADERS
    src/MainWindow.h
    src/GridGraphicsView.h
    src/ControlPanel.h
    src/ResultListWidget.h
    src/PathCalculator.h
    src/DataManager.h
    src/HistoryManager.h
    src/GridPoint.h
    src/PathResult.h
    src/Common.h
)

# Create executable
qt6_add_executable(QWeight_demo ${SOURCES} ${HEADERS})

# Set target properties for MOC
set_target_properties(QWeight_demo PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Link Qt6 libraries
target_link_libraries(QWeight_demo PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Sql
    Qt6::Xml
    Qt6::Concurrent
)
