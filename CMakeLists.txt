cmake_minimum_required(VERSION 3.16)

project(QtStmAutomotiveSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wconversion -Wsign-conversion -Werror=return-type)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick Test SerialPort Multimedia)

qt_standard_project_setup(REQUIRES 6.8)

# Add tests directory
enable_testing()
add_subdirectory(tests)

qt_add_executable(${PROJECT_NAME}
    src/main.cpp
    src/services/SimulatorService.cpp
    src/services/MockScenarioEngine.cpp
    src/services/SerialService.cpp
    src/services/MusicScanner.cpp
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(${PROJECT_NAME}
    URI "com.showcase"
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/components/GlowingText.qml
        qml/components/NeonTickGauge.qml
        qml/components/MusicPlayer.qml
        qml/components/EnergyBlocks.qml
        qml/components/GlassPanel.qml
        qml/components/NeonIcon.qml
        qml/components/NeonIconButton.qml
        qml/components/RangeTripCard.qml
        qml/components/NeonMapView.qml
        qml/components/CenterHub.qml
        qml/screens/DashboardScreen.qml
    RESOURCES
        resources/icons/play.svg
        resources/icons/pause.svg
        resources/icons/next.svg
        resources/icons/prev.svg
        resources/icons/shuffle.svg
        resources/icons/repeat.svg
        resources/icons/volume.svg
        resources/icons/warning-triangle.svg
        resources/icons/battery-low.svg
        resources/icons/temperature-high.svg
        resources/icons/theme-day.svg
        resources/icons/theme-night.svg
        resources/icons/vehicle-car.svg
        resources/icons/vehicle-scooter.svg
        resources/icons/vehicle-bike.svg
        resources/icons/drive-eco.svg
        resources/icons/drive-normal.svg
        resources/icons/drive-sport.svg
    SOURCES
        src/viewmodels/VehicleStatusViewModel.h
        src/viewmodels/VehicleStatusViewModel.cpp
        src/viewmodels/MusicPlayerViewModel.h
        src/viewmodels/MusicPlayerViewModel.cpp
        src/viewmodels/ThemeViewModel.h
        src/viewmodels/ThemeViewModel.cpp
        src/viewmodels/VehicleModeViewModel.h
        src/viewmodels/VehicleModeViewModel.cpp
        src/viewmodels/DriveModeViewModel.h
        src/viewmodels/DriveModeViewModel.cpp
        src/viewmodels/TripComputerViewModel.h
        src/viewmodels/TripComputerViewModel.cpp
        src/viewmodels/MapViewModel.h
        src/viewmodels/MapViewModel.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::SerialPort Qt6::Multimedia
)

target_include_directories(${PROJECT_NAME} PRIVATE src src/viewmodels)

# Standard install paths
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
