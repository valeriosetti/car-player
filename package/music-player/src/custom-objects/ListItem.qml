import QtQuick 2.0
import QtQuick.Controls 2.15

ItemDelegate {
    id: control
    property int text_size : 16

    highlighted: ListView.isCurrentItem

    contentItem: Text {
        text: model.item_name

        color: control.highlighted ? "green" : "white"
        font.bold: control.highlighted
        font.pixelSize: control.highlighted ? control.text_size*1.5 : control.text_size

        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        opacity: control.highlighted ? 0.2 : 0.0
        color: "white"
        radius: 15
    }
}
