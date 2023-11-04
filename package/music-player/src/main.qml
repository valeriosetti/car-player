import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "custom-objects"

Window {
    width: 800
    height: 480
    visible: true
    title: "Car player"

    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width

    id: main_window

    Loader {
        id: mainLoader
        anchors.fill: parent
        source: "qrc:/main-menu-ui.qml"
    }
}
