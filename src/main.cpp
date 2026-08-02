#include "services/SerialService.h"
#include "services/SimulatorService.h"
#include "services/MockParkingSensorService.h"
#include "services/TelemetryMapper.h"
#include "viewmodels/VehicleStatusViewModel.h"
#include "viewmodels/MusicPlayerViewModel.h"
#include "viewmodels/ThemeViewModel.h"
#include "viewmodels/TripComputerViewModel.h"
#include "viewmodels/ParkingAssistViewModel.h"
#include "viewmodels/CenterHubViewModel.h"
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlError>
#include <QWindow>

int main(int argc, char *argv[]) {
  // Qt Creator/debug environments may promote qWarning() to process
  // termination through QT_FATAL_WARNINGS. Optional graphics backend probes
  // must never make this dashboard exit before the UI is created.
  qunsetenv("QT_FATAL_WARNINGS");

  // The dashboard uses QMediaPlayer for audio. Disable optional FFmpeg video
  // hardware probing so a missing VDPAU backend cannot abort startup on AMD
  // systems; audio playback remains unchanged and future hardware can opt in
  // through the standard QT_FFMPEG_DECODING_HW_DEVICE_TYPES environment setting.
  if (!qEnvironmentVariableIsSet("QT_FFMPEG_DECODING_HW_DEVICE_TYPES")) {
    qputenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES", QByteArrayLiteral(","));
  }

  QGuiApplication app(argc, argv);

  VehicleStatusViewModel vm;
  TripComputerViewModel tripVm;
  MockParkingSensorService parkingSensorService;
  ParkingAssistViewModel parkingAssistVm;
  CenterHubViewModel centerHubVm(&parkingAssistVm);
  ThemeViewModel themeVm;
  QElapsedTimer tripClock;
  tripClock.start();

  // Setup both services
  SimulatorService simulatorService;
  SerialService serialService("/dev/ttyUSB0");
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

  QObject::connect(&parkingSensorService,
                   &MockParkingSensorService::parkingSampleUpdated,
                   &parkingAssistVm,
                   &ParkingAssistViewModel::updateSensorSample);

  // Start Serial Service by default. It will emit connectionStatusChanged(false) if it fails,
  // triggering the SimulatorService to start as a fallback.
  serialService.startService();

  QQmlApplicationEngine engine;

  QObject::connect(&engine, &QQmlEngine::warnings, &app,
                   [](const QList<QQmlError> &warnings) {
                       for (const QQmlError &warning : warnings) {
                           qCritical().noquote() << warning.toString();
                       }
                   });

  // Expose ViewModels to QML
  MusicPlayerViewModel musicVm;
  engine.rootContext()->setContextProperty("VehicleStatus", &vm);
  engine.rootContext()->setContextProperty("MusicViewModel", &musicVm);
  engine.rootContext()->setContextProperty("ThemeController", &themeVm);
  engine.rootContext()->setContextProperty("TripComputer", &tripVm);
  engine.rootContext()->setContextProperty("ParkingAssist", &parkingAssistVm);
  engine.rootContext()->setContextProperty("CenterHubController", &centerHubVm);

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
      [](const QUrl &url) {
          qCritical().noquote() << "QML object creation failed:" << url.toString();
          QCoreApplication::exit(-1);
      }, Qt::QueuedConnection);

  engine.loadFromModule("com.showcase", "Main");

  parkingSensorService.start();

  return app.exec();
}
