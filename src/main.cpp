#include "services/SerialService.h"
#include "services/SimulatorService.h"
#include "viewmodels/VehicleStatusViewModel.h"
#include "viewmodels/MusicPlayerViewModel.h"
#include "viewmodels/ThemeViewModel.h"
#include "viewmodels/VehicleModeViewModel.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  VehicleStatusViewModel vm;

  // Setup both services
  SimulatorService simulatorService;
  SerialService serialService("/dev/ttyUSB0");

  bool isHardwareConnected = false;

  // Handle hardware connection status
  QObject::connect(&serialService, &SerialService::connectionStatusChanged, [&](bool connected) {
      isHardwareConnected = connected;
      if (connected) {
          qDebug() << "Hardware connected! Using SerialService.";
          simulatorService.stopSimulation();
      } else {
          qDebug() << "Hardware disconnected. Falling back to SimulatorService.";
          simulatorService.startSimulation();
      }
  });

  // Route telemetry from SerialService
  QObject::connect(&serialService, &SerialService::telemetryUpdated, [&](double speed, int rpm, const QString &gear, bool isWarning, int battery, int range, int temperature) {
      if (isHardwareConnected) {
          vm.updateTelemetry(speed, rpm, gear, isWarning, battery, range, temperature);
      }
  });

  // Route telemetry from SimulatorService
  QObject::connect(&simulatorService, &SimulatorService::telemetryUpdated, [&](double speed, int rpm, const QString &gear, bool isWarning, int battery, int range, int temperature) {
      if (!isHardwareConnected) {
          vm.updateTelemetry(speed, rpm, gear, isWarning, battery, range, temperature);
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
  engine.rootContext()->setContextProperty("VehicleStatus", &vm);
  engine.rootContext()->setContextProperty("MusicViewModel", &musicVm);
  engine.rootContext()->setContextProperty("ThemeController", &themeVm);
  engine.rootContext()->setContextProperty("VehicleMode", &vehicleModeVm);

  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("com.showcase", "Main");

  themeVm.startBootSequence();

  return app.exec();
}
