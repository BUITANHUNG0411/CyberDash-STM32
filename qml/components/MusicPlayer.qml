import QtQuick
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Effects
import com.showcase

Item {
    id: root
    implicitWidth: 350
    implicitHeight: 450

    // Neon Blur Backdrop driven by current song color1 (Zero-JS)
    Rectangle {
        id: blurBackdrop
        anchors.fill: parent
        color: Theme.backgroundDeepSpace
        opacity: 0.65

        // Source ẩn cho hiệu ứng blur — MultiEffect phải là SIBLING của source,
        // tuyệt đối không dùng source: parent (đệ quy ShaderEffectSource → feedback loop).
        Item {
            id: backdropSource
            anchors.fill: parent
            visible: false
            Image {
                anchors.fill: parent
                source: MusicViewModel.currentCoverArt
                fillMode: Image.PreserveAspectCrop
                visible: source.toString() !== ""
            }
            Rectangle {
                anchors.fill: parent
                visible: parent.children[0].source.toString() === ""
                gradient: Gradient {
                    GradientStop { position: 0.0; color: pathView.currentItem !== null ? (pathView.currentItem.coverColor1 !== undefined ? pathView.currentItem.coverColor1 : Theme.coverFallback1) : Theme.coverFallback1 }
                    GradientStop { position: 1.0; color: Theme.backgroundDeepSpace }
                }
            }
        }

        MultiEffect {
            anchors.fill: parent
            source: backdropSource
            blurEnabled: true
            blurMax: 48
            blur: 1.0
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spaceLg

        // Top 70%: Cover Flow Area
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            PathView {
                id: pathView
                anchors.fill: parent
                model: MusicViewModel
                pathItemCount: 3
                preferredHighlightBegin: 0.5
                preferredHighlightEnd: 0.5
                interactive: true
                
                // Pure declarative binding instead of Connections
                currentIndex: MusicViewModel.currentIndex

                onMovementEnded: MusicViewModel.setCurrentIndex(currentIndex)

                // 3D Depth Path
                path: Path {
                    startX: 0
                    startY: pathView.height / 2

                    PathAttribute { name: "itemZ"; value: 0 }
                    PathAttribute { name: "itemScale"; value: 0.6 }
                    PathAttribute { name: "itemOpacity"; value: 0.4 }

                    PathLine { x: pathView.width / 2; y: pathView.height / 2 }
                    PathAttribute { name: "itemZ"; value: 100 }
                    PathAttribute { name: "itemScale"; value: 1.0 }
                    PathAttribute { name: "itemOpacity"; value: 1.0 }

                    PathLine { x: pathView.width; y: pathView.height / 2 }
                    PathAttribute { name: "itemZ"; value: 0 }
                    PathAttribute { name: "itemScale"; value: 0.6 }
                    PathAttribute { name: "itemOpacity"; value: 0.4 }
                }

                delegate: Item {
                    width: pathView.width * 0.55
                    height: width
                    scale: PathView.itemScale
                    opacity: PathView.itemOpacity
                    z: PathView.itemZ

                    // Expose the roles used by the backdrop and cover image.
                    property string coverColor1: model.color1
                    property string coverArt: model.coverArt !== undefined ? model.coverArt : ""

                    // Base container for the cover (rotates when playing)
                    Rectangle {
                        id: coverRect
                        anchors.fill: parent
                        radius: Theme.radiusMd
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: model.color1 !== undefined ? model.color1 : Theme.coverFallback1 }
                            GradientStop { position: 1.0; color: model.color2 !== undefined ? model.color2 : Theme.coverFallback2 }
                        }
                        border.color: PathView.isCurrentItem ? Theme.accentCyan : "transparent"
                        border.width: PathView.isCurrentItem ? 2 : 0
                        clip: true

                        Image {
                            anchors.fill: parent
                            source: parent.parent.coverArt
                            fillMode: Image.PreserveAspectCrop
                            visible: source.toString() !== ""
                        }

                        transform: Rotation {
                            id: coverRotation
                            origin.x: coverRect.width / 2
                            origin.y: coverRect.height / 2
                            axis { x: 0; y: 1; z: 0 }

                            NumberAnimation on angle {
                                from: 0
                                to: 360
                                duration: Theme.durationCover
                                loops: Animation.Infinite
                                running: PathView.isCurrentItem && MusicViewModel.isPlaying
                            }
                        }

                        // Reflection mask effect fading into background
                        Rectangle {
                            anchors.fill: parent
                            radius: Theme.radiusMd
                            gradient: Gradient {
                                GradientStop { position: 0.5; color: "transparent" }
                                GradientStop { position: 1.0; color: Theme.backgroundDeepSpace }
                            }
                        }
                    }

                    // Neon Glow Effect for the current item
                    MultiEffect {
                        anchors.fill: coverRect
                        source: coverRect
                        shadowEnabled: PathView.isCurrentItem
                        shadowColor: Theme.accentCyan
                        shadowBlur: 1.0
                        shadowHorizontalOffset: 0
                        shadowVerticalOffset: 0
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: MusicViewModel.play(index)
                    }
                }
            }

            // Loading spinner overlay (runs only while isLoading)
            ColumnLayout {
                anchors.centerIn: parent
                spacing: Theme.spaceMd
                visible: MusicViewModel.isLoading

                Item {
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64

                    Rectangle {
                        anchors.fill: parent
                        radius: Theme.radiusPill
                        color: "transparent"
                        border.color: Theme.accentCyan
                        border.width: 3
                        opacity: 0.25
                    }

                    Item {
                        anchors.fill: parent
                        id: spinnerArc
                        
                        Rectangle {
                            width: parent.width
                            height: parent.height
                            radius: Theme.radiusPill
                            color: "transparent"
                            border.color: Theme.accentCyan
                            border.width: 3
                            
                            // A hacky way to create an arc using gradients or clip, but we stick to the existing visual
                            // By default, the old code had a visible: false rectangle that was rotated. We'll just use a small indicator.
                            clip: true
                            Rectangle {
                                width: parent.width / 2
                                height: parent.height / 2
                                color: Theme.backgroundDeepSpace
                                anchors.top: parent.top
                                anchors.right: parent.right
                            }
                        }

                        transform: Rotation {
                            origin.x: 32
                            origin.y: 32
                            NumberAnimation on angle {
                                from: 0
                                to: 360
                                duration: Theme.durationSpin
                                loops: Animation.Infinite
                                running: MusicViewModel.isLoading
                            }
                        }
                    }
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Loading..."
                    color: Theme.accentCyan
                    font.family: Theme.fontMain
                    font.pixelSize: Theme.textXs
                }
            }
        }

        // Bottom Navigation Control Panel
        GlassPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: 176

            Column {
                anchors.centerIn: parent
                width: parent.width * 0.85
                spacing: Theme.spaceMd

                // 1. Track Info (Centered) & Scan Button (Right)
                Item {
                    width: parent.width
                    height: 48

                    Column {
                        id: trackInfo
                        spacing: 2
                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: scanButton.width + Theme.spaceMd
                            rightMargin: scanButton.width + Theme.spaceMd
                            verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: MusicViewModel.currentTitle !== "" ? MusicViewModel.currentTitle : (MusicViewModel.isScanning ? "Scanning..." : "No Music")
                            font.family: Theme.fontMain
                            font.pixelSize: Theme.textMd
                            font.bold: true
                            color: Theme.textPrimary
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                        }
                        Text {
                            text: MusicViewModel.currentArtist
                            font.family: Theme.fontMain
                            font.pixelSize: Theme.textXs
                            color: Theme.textSecondary
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                        }
                    }

                    Rectangle {
                        id: scanButton
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        width: 60
                        height: 28
                        radius: Theme.radiusSm
                        color: MusicViewModel.isScanning ? Theme.accentCyan : "transparent"
                        border.color: Theme.accentCyan
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: MusicViewModel.isScanning ? "..." : "Scan"
                            color: MusicViewModel.isScanning ? Theme.backgroundDeepSpace : Theme.accentCyan
                            font.family: Theme.fontMain
                            font.pixelSize: Theme.textXs
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: MusicViewModel.scanLibrary()
                            enabled: !MusicViewModel.isScanning
                        }
                    }
                }

                // 2. Scrubber
                Item {
                    width: parent.width
                    height: 12
                    anchors.horizontalCenter: parent.horizontalCenter

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width
                        height: 3
                        color: Theme.trackInactive
                        radius: 1.5

                        Rectangle {
                            width: parent.width * (MusicViewModel.scrubberDragging
                                ? MusicViewModel.scrubberRatio
                                : MusicViewModel.progress)
                            height: parent.height
                            color: Theme.accentCyan
                            radius: 1.5

                            Rectangle {
                                id: thumb
                                width: 10
                                height: 10
                                radius: 5
                                color: Theme.textPrimary
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: -5
                            }
                            MultiEffect {
                                anchors.fill: thumb
                                source: thumb
                                shadowEnabled: true
                                shadowColor: Theme.accentCyan
                                shadowBlur: 1.0
                            }
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        onPressed: MusicViewModel.beginScrub(mouseX, width)
                        onPositionChanged: MusicViewModel.updateScrub(mouseX, width)
                        onReleased: MusicViewModel.endScrub()
                        onCanceled: MusicViewModel.endScrub()
                    }
                }

                // 3. Volume Slider
                Row {
                    width: parent.width
                    height: 16
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.spaceMd

                    Item {
                        width: 16
                        height: 16
                        anchors.verticalCenter: parent.verticalCenter

                        NeonIcon {
                            anchors.fill: parent
                            source: "qrc:/qt/qml/com/showcase/resources/icons/volume.svg"
                            sourceSize: Qt.size(16, 16)
                            colorizationColor: Theme.textSecondary
                        }
                    }

                    Rectangle {
                        width: parent.width - 16 - 8
                        anchors.verticalCenter: parent.verticalCenter
                        height: 3
                        color: Theme.trackInactive
                        radius: 1.5

                        Rectangle {
                            width: parent.width * MusicViewModel.volume
                            height: parent.height
                            color: Theme.accentCyan
                            radius: 1.5
                        }

                        MouseArea {
                            anchors.fill: parent
                            onPressed: MusicViewModel.setVolumeFromPosition(mouseX, width)
                            onPositionChanged: MusicViewModel.setVolumeFromPosition(mouseX, width)
                        }
                    }
                }

                // 4. Media Controls
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: Theme.spaceXl

                    NeonIconButton {
                        width: 24
                        height: 24
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/qt/qml/com/showcase/resources/icons/shuffle.svg"
                        sourceSize: Qt.size(24, 24)
                        isActive: MusicViewModel.shuffleMode
                        defaultColor: Theme.textSecondary
                        activeColor: Theme.accentCyan
                        onClicked: MusicViewModel.toggleShuffle()
                    }

                    NeonIconButton {
                        width: 26
                        height: 26
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/qt/qml/com/showcase/resources/icons/prev.svg"
                        sourceSize: Qt.size(26, 26)
                        defaultColor: Theme.textPrimary
                        onClicked: MusicViewModel.prev()
                    }

                    Rectangle {
                        width: 44
                        height: 44
                        radius: Theme.radiusLg
                        anchors.verticalCenter: parent.verticalCenter
                        color: playMouse.pressed ? Theme.accentCyan : "transparent"
                        border.color: Theme.accentCyan
                        border.width: 2
                        scale: playMouse.pressed ? 0.9 : 1.0
                        Behavior on scale { NumberAnimation { duration: Theme.durationPress } }
                        Behavior on color { ColorAnimation { duration: Theme.durationPress } }
                        
                        NeonIcon {
                            width: 20
                            height: 20
                            anchors.centerIn: parent
                            source: MusicViewModel.isPlaying ? "qrc:/qt/qml/com/showcase/resources/icons/pause.svg" : "qrc:/qt/qml/com/showcase/resources/icons/play.svg"
                            sourceSize: Qt.size(20, 20)
                            colorizationColor: playMouse.pressed ? Theme.backgroundDeepSpace : Theme.accentCyan
                        }
                        
                        MouseArea { id: playMouse; anchors.fill: parent; onClicked: MusicViewModel.togglePlayPause() }
                    }

                    NeonIconButton {
                        width: 26
                        height: 26
                        anchors.verticalCenter: parent.verticalCenter
                        source: "qrc:/qt/qml/com/showcase/resources/icons/next.svg"
                        sourceSize: Qt.size(26, 26)
                        defaultColor: Theme.textPrimary
                        onClicked: MusicViewModel.next()
                    }

                    Item {
                        width: 24
                        height: 24
                        anchors.verticalCenter: parent.verticalCenter
                        
                        NeonIconButton {
                            anchors.fill: parent
                            source: "qrc:/qt/qml/com/showcase/resources/icons/repeat.svg"
                            sourceSize: Qt.size(24, 24)
                            defaultColor: Theme.textSecondary
                            activeColor: MusicViewModel.repeatMode === MusicEnums.RepeatMode.One ? Theme.warningRed : Theme.accentCyan
                            isActive: MusicViewModel.repeatMode !== MusicEnums.RepeatMode.Off
                            onClicked: MusicViewModel.cycleRepeat()
                        }
                        
                        Rectangle {
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            width: 10
                            height: 10
                            radius: 5
                            color: Theme.warningRed
                            visible: MusicViewModel.repeatMode === MusicEnums.RepeatMode.One
                        }
                    }
                }
            }
        }
    }

    // Error Toast (driven by declarative binding to MusicViewModel.lastError)
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: parent.width - 24 < toastText.width + 28
            ? parent.width - 24
            : toastText.width + 28
        height: 34
        radius: Theme.radiusSm
        color: Theme.warningRed
        opacity: MusicViewModel.lastError !== "" ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: Theme.durationNormal } }

        Text {
            id: toastText
            anchors.centerIn: parent
            text: MusicViewModel.lastError
            color: Theme.textPrimary
            font.family: Theme.fontMain
            font.pixelSize: Theme.textXs
            elide: Text.ElideRight
        }
        
        Timer {
            interval: 3000
            running: MusicViewModel.lastError !== ""
            repeat: false
            onTriggered: MusicViewModel.clearError()
        }
    }
}
