import QtQuick 2.0
import QtQuick.Controls 2.15

Button {
    id: _button
    property string idle_background : ""
    property string pressed_background : ""
    property string button_icon : ""

    width: 64
    height: 64
    implicitWidth: 64
    implicitHeight: 64

    background: Image {
        id: background_image
        source: _button.down ? pressed_background : idle_background
        height: parent.height
        width: parent.width
    }

    contentItem: Image {
        source: button_icon
        anchors.verticalCenter: background_image.verticalCenter
        anchors.horizontalCenter: background_image.horizontalCenter
        height: background_image.height * 0.75
        width: background_image.width * 0.75
    }
}
