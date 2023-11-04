import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "custom-objects"

Item {
    Image {
        source: "images/background.png"
    }

    ColumnLayout {
        anchors.fill: parent
        width: parent.width
        height: parent.height

        MainMenuButton {
            objectName: "radio-button"
            button_icon: "qrc:/images/radio-icon.png"
            button_text: "Radio"
            Layout.alignment: Qt.AlignHCenter
        }

        MainMenuButton {
            objectName: "usb-button"
            button_icon: "qrc:/images/usb-icon.png"
            button_text: "USB stick"
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                mainLoader.source = "qrc:/music-player-ui.qml"
            }
        }

        MainMenuButton {
            objectName: "configuration-button"
            button_icon: "qrc:/images/config-icon.png"
            button_text: "Settings"
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
