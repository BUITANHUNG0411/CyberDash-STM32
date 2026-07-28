pragma ComponentBehavior: Bound

import QtQuick
import com.showcase

/*
 * Hub trung tâm (Car mode): giữ MusicPlayer sống ổn định trong panel.
 */
Item {
    id: root

    MusicPlayer {
        anchors.fill: parent
    }
}
