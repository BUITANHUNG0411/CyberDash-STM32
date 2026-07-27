import QtQuick
import QtQuick.Controls.Basic
import com.showcase

/*
 * Hub trung tâm lướt được (Car mode): Music ⇄ Road.
 * SwipeView giữ children tĩnh sống mãi — MusicPlayer không bao giờ bị hủy.
 */
Item {
    id: root

    SwipeView {
        id: swipeView
        anchors.fill: parent
        clip: true

        MusicPlayer {}
        PerspectiveRoadView {}
    }

    PageIndicator {
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        anchors.top: parent.top
        anchors.topMargin: Theme.spaceSm
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Theme.spaceMd
        delegate: Rectangle {
            required property int index
            width: 8
            height: 8
            radius: 4
            color: index === swipeView.currentIndex ? Theme.accentCyan : Theme.trackInactive
            Behavior on color { ColorAnimation { duration: Theme.durationFast } }
        }
    }
}
