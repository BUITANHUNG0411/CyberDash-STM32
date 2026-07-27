#include "services/SerialService.h"
#include "services/SimulatorService.h"
#include "services/MockWheelTelemetryService.h"
#include "services/TelemetryMapper.h"
#include "viewmodels/VehicleStatusViewModel.h"
#include "viewmodels/MusicPlayerViewModel.h"
#include "viewmodels/ThemeViewModel.h"
#include "viewmodels/VehicleModeViewModel.h"
#include "viewmodels/DriveModeViewModel.h"
#include "viewmodels/EncoderDriveViewModel.h"
#include "viewmodels/TripComputerViewModel.h"
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWindow>

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  VehicleStatusViewModel vm;
  TripComputerViewModel tripVm;
  EncoderDriveViewModel encoderDriveVm;
  QElapsedTimer tripClock;
  tripClock.start();

  // Setup both services
  SimulatorService simulatorService;
  SerialService serialService("/dev/ttyUSB0");
  MockWheelTelemetryService mockWheelTelemetry;
  QObject::connect(
      &mockWheelTelemetry,
      &MockWheelTelemetryService::wheelTelemetryUpdated,
      &encoderDriveVm,
      &EncoderDriveViewModel::updateWheelMotion);

  bool isHardwareConnected = false;

  // Handle hardware connection status
  QObject::connect(&serialService, &SerialService::connectionStatusChanged, [&](bool connected) {
      isHardwareConnected = connected;
      tripClock.restart(); // drop the dead-time gap when the telemetry source switches
      if (connected) {
          qDebug() << "Hardware connected! Using SerialService.";
          simulatorService.stopSimulation();
      } else {
          qDebug() << "Hardware disconnected. Falling back to SimulatorService.";
          simulatorService.startSimulation();
      }
  });

  // Route raw telemetry from SerialService through the shared dashboard mapper.
  QObject::connect(&serialService, &SerialService::rawTelemetryUpdated,
                   [&](int rpm, double batteryVoltage, int errorCode) {
      if (!isHardwareConnected) {
          return;
      }
      const DashboardTelemetry data =
          TelemetryMapper::fromSerial({rpm, batteryVoltage, errorCode});
      vm.updateTelemetry(data.speed, data.rpm, data.gear, data.warning,
                         data.battery, data.range, data.temperature);
      tripVm.updateSpeed(data.speed, tripClock.restart()); // restart() returns elapsed ms
  });

  // Route telemetry from SimulatorService
  QObject::connect(&simulatorService, &SimulatorService::telemetryUpdated, [&](double speed, int rpm, const QString &gear, bool isWarning, int battery, int range, int temperature) {
      if (!isHardwareConnected) {
          vm.updateTelemetry(speed, rpm, gear, isWarning, battery, range, temperature);
          tripVm.updateSpeed(speed, tripClock.restart()); // restart() returns elapsed ms
      }
  });

  // Start Serial Service by default. It will emit connectionStatusChanged(false) if it fails,
  // triggering the SimulatorService to start as a fallback.
  serialService.startService();

  QQmlApplicationEngine engine;

  // Expose ViewModels to QML
  MusicPlayerViewModel musicVm;
  ThemeViewModel themeVm;
  VehicleModeViewModel vehicleModeVm;
  DriveModeViewModel driveModeVm;
  engine.rootContext()->setContextProperty("VehicleStatus", &vm);
  engine.rootContext()->setContextProperty("MusicViewModel", &musicVm);
  engine.rootContext()->setContextProperty("ThemeController", &themeVm);
  engine.rootContext()->setContextProperty("VehicleMode", &vehicleModeVm);
  engine.rootContext()->setContextProperty("DriveMode", &driveModeVm);
  engine.rootContext()->setContextProperty("TripComputer", &tripVm);
  engine.rootContext()->setContextProperty("EncoderDrive", &encoderDriveVm);

  QObject::connect(&themeVm, &ThemeViewModel::windowMoveRequested, &engine, [&engine]() {
      const auto rootObjects = engine.rootObjects();
      if (rootObjects.isEmpty()) {
          return;
      }
      if (auto *window = qobject_cast<QWindow *>(rootObjects.constFirst())) {
          window->startSystemMove();
      }
  });

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("com.showcase", "Main");

  themeVm.startBootSequence();
  mockWheelTelemetry.start();

  return app.exec();
}
