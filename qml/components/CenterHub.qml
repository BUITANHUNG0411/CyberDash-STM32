pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import com.showcase

/*
 * Hub trung tâm lướt được (Car mode): Music ⇄ Encoder Drive.
 * SwipeView giữ children tĩnh sống mãi — MusicPlayer không bao giờ bị hủy.
 */
Item {
    id: root

    SwipeView {
        id: swipeView
        anchors.fill: parent
        clip: true

        MusicPlayer {}
        EncoderDriveView {}
    }

    Component {
        id: pageIndicatorDelegate

        Rectangle {
            id: pageDot

            required property int index

            width: 8
            height: 8
            radius: 4
            color: pageDot.index === swipeView.currentIndex
                   ? Theme.accentCyan
                   : Theme.trackInactive

            Behavior on color {
                ColorAnimation {
                    duration: Theme.durationFast
                }
            }
        }
    }

    PageIndicator {
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        spacing: Theme.spaceMd
        delegate: pageIndicatorDelegate

        anchors {
            top: parent.top
            topMargin: Theme.spaceSm
            horizontalCenter: parent.horizontalCenter
        }
    }
}
