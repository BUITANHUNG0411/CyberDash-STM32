pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import com.showcase

/*
 * Hub trung tâm (Car mode): giữ MusicPlayer sống ổn định trong panel.
 */
Item {
    id: root

    StackLayout {
        anchors.fill: parent
        currentIndex: CenterHubController.activePage

        MusicPlayer {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ParkingAssistView {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    // The hub owns horizontal navigation; child MouseAreas still receive clicks,
    // while a real drag can take over and is committed by the C++ ViewModel.
    DragHandler {
        id: hubSwipeHandler
        target: null
        grabPermissions: PointerHandler.CanTakeOverFromAnything

        onActiveChanged: CenterHubController.setSwipeActive(active)
        onTranslationChanged: CenterHubController.updateSwipeTranslation(translation.x)
    }
}
