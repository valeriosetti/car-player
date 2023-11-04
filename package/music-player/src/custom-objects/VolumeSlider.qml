import QtQuick 2.0
import QtQuick.Controls 2.15

Slider {
    id: slider
    from: 0
    to: 100

    property double background_start_point

    background: Canvas {
        onPaint: {
            var ctx = getContext("2d")
            ctx.fillStyle = "white"
            ctx.beginPath()
            ctx.moveTo(0, 0)
            ctx.lineTo(slider.width, 0)
            ctx.lineTo(0, slider.height)
            ctx.closePath()
            ctx.fill()
        }
    }

    handle: Rectangle {
        id: handle
        width: slider.width
        height: 10
        color: "green"
        x: 0
        y: background_start_point
    }

    onValueChanged: {
        background_start_point = slider.visualPosition * (slider.height - handle.height)
    }
}
