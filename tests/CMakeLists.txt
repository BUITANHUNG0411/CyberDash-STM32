# tests/CMakeLists.txt

# Create a test executable
qt_add_executable(tst_viewmodels
    main.cpp
    ../src/viewmodels/VehicleStatusViewModel.cpp
    ../src/viewmodels/ThemeViewModel.cpp
    ../src/viewmodels/VehicleModeViewModel.cpp
    ../src/viewmodels/DriveModeViewModel.cpp
    ../src/viewmodels/TripComputerViewModel.cpp
    ../src/viewmodels/MapViewModel.cpp
)

target_include_directories(tst_viewmodels PRIVATE ../src)

# Link against Qt Test and our application's libraries if needed
# We also link against Core to ensure it has basic functionality
target_link_libraries(tst_viewmodels
    PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Test
)

# Add it to CTest
add_test(NAME tst_viewmodels COMMAND tst_viewmodels)

# --- Music playback tests ---
qt_add_executable(tst_music_playback
    tst_music_playback.cpp
    ../src/viewmodels/MusicPlayerViewModel.cpp
    ../src/services/MusicScanner.cpp
)

target_include_directories(tst_music_playback PRIVATE ../src)

target_link_libraries(tst_music_playback
    PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Test
    Qt6::Multimedia
)

add_test(NAME tst_music_playback COMMAND tst_music_playback)
set_tests_properties(tst_music_playback PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    TIMEOUT 20
)

qt_add_executable(tst_serial_pipeline
    tst_serial_pipeline.cpp
    ../src/services/SerialService.cpp
    ../src/services/SerialTelemetryParser.cpp
    ../src/services/TelemetryMapper.cpp
)

target_include_directories(tst_serial_pipeline PRIVATE ../src)

target_link_libraries(tst_serial_pipeline
    PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::SerialPort
)

add_test(NAME tst_serial_pipeline COMMAND tst_serial_pipeline)

qt_add_executable(tst_road_motion
    tst_road_motion.cpp
    ../src/services/MockWheelTelemetryService.cpp
)

target_include_directories(tst_road_motion PRIVATE ../src)

target_link_libraries(tst_road_motion
    PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME tst_road_motion COMMAND tst_road_motion)
