# Qt Creator VDPAU Runtime Exit Research

> **AI Context**: Environment-specific investigation for the Qt 6.11 dashboard on an AMD Linux desktop. No GPU driver, package, or host configuration was changed.

## Executive finding

The project compiled successfully. Qt Creator was launching the GUI binary, which
terminated immediately after a VDPAU warning with exit code `255`. The warning is
an optional video-backend probe; it is not a C++ compile/link error. Qt documents
that `QT_FATAL_WARNINGS` can terminate a process on warnings. A separate QML
compile error in `CenterHub.qml` (`DragHandler` has no `anchors` property) was later
found and fixed independently.

## In-repository mitigation

`src/main.cpp` removes `QT_FATAL_WARNINGS` for this process and sets
`QT_FFMPEG_DECODING_HW_DEVICE_TYPES=,` when no explicit override exists. This
disables optional FFmpeg video hardware probing while retaining audio playback.
`MusicPlayerViewModel` keeps `QAudioOutput` available for the QML object graph and
defers `QMediaPlayer` construction until a real play request. These changes are
process-local and do not install, remove, or alter host GPU drivers.

The independent QML failure was corrected by removing `anchors.fill: parent` from
the `DragHandler`; pointer handlers use their containing Item as the event scope.

## Sources

- [Qt Advanced FFmpeg Configuration](https://doc.qt.io/qt-6/advanced-ffmpeg-configuration.html)
- [Qt QDebug](https://doc.qt.io/qt-6/qdebug.html)
- [Qt DragHandler](https://doc.qt.io/qt-6/qml-qtquick-draghandler.html)

## Verification

Configure/build, CTest `4/4`, QML lint, and Desktop Debug offscreen smoke pass.
The smoke run reaches the expected timeout (`124`) with only the known PulseAudio
permission warning and no QML object-creation failure.
