import QtQuick 2.0
import QtQuick.Controls 2.15

Button {
    id: thisButton

    property string button_icon : ""
    property string button_text : ""

    implicitWidth: parent.width * 0.75
    implicitHeight: (parent.height - 150)/3

    background: Rectangle {
        id: outsideRectangle
        color: Qt.rgba(0,0,0,0)
        border.color: thisButton.down ? "blue" : "white"
        border.width: 3
        radius: 20
        width: thisButton.width
        height: thisButton.height
    }

    contentItem: Item {
        id: insideItem
        anchors.centerIn: outsideRectangle
        anchors.fill: outsideRectangle

        Image {
            id: buttonIcon
            source: button_icon
            height: thisButton.height * 0.8
            fillMode: Image.PreserveAspectFit
            anchors.verticalCenter: insideItem.verticalCenter
            anchors.left: insideItem.left
            anchors.leftMargin: 100
        }
        Text {
            id: buttonText
            text: button_text
            color: "white"
            font.pointSize: 36
            anchors.left: buttonIcon.right
            anchors.leftMargin: 50
            anchors.verticalCenter: insideItem.verticalCenter
        }
    }
}
