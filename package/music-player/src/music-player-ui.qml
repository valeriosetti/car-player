import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "custom-objects"
import cpp_custom_components 1.0

Item {
    id: music_player_ui

    Image {
        source: "images/background.png"
    }

    focus: true
    Keys.onLeftPressed: { songListView.decrementCurrentIndex() }
    Keys.onRightPressed: { songListView.incrementCurrentIndex() }
    Keys.onEscapePressed: { Qt.quit() }

    ColumnLayout {
        id: columnLayout
        anchors.fill: parent
        anchors.rightMargin: 10
        anchors.leftMargin: 10
        anchors.topMargin: 10

        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true

            ScrollView {
                id: songScrollView
                Layout.fillHeight: true
                Layout.fillWidth: true

                clip: true

                ListView {
                    id: songListView
                    model: fileListModel

                    delegate: ListItem {
                        width: songScrollView.width
                        onClicked: {
                            songListView.currentIndex = index
                        }
                    }

                    onCurrentItemChanged: {
                        gstManager.play(fileListModel.get_id_for_index((songListView.currentIndex)))
                    }

                    Connections {
                        target: gstManager
                        function onTrack_eos() { songListView.incrementCurrentIndex() }
                    }
                }
            }

            ColumnLayout {
                Layout.fillHeight: true
                SpectrumGraph {
                    id: spectrumGraph
                    objectName: "spectrumGraph"
                    width: 300
                    Layout.fillHeight: true
                    bars_count: 20
                    Component.onCompleted: {
                        gstManager.set_spectrum_bars_count(bars_count)
                        gstManager.set_spectrum_graph_ptr(spectrumGraph)
                    }
                }

                Text {
                    id: time_text
                    text: gstManager.playback_time
                    color: "white"
                    font.pointSize: 30
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 20
                VolumeSlider {
                    id: volumeSlider
                    orientation: "Vertical"
                    Layout.preferredWidth: 20
                    Layout.fillHeight: true
                    value: 75
                }
                Image {
                    source: "qrc:/images/volume-icon.png"
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                }
            }
        }

        Slider {
            id: elapsedTimerSlider
            Layout.fillWidth: true
            from: 0
            to: 1000
            value: (pressed) ? value : gstManager.progress
            onPressedChanged: {
                if (!pressed) {
                    gstManager.seek(value)
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.bottomMargin: 10

            CustomButton {
                idle_background: "qrc:/images/player-background-idle.png"
                pressed_background: "qrc:/images/player-background-pressed.png"
                button_icon: "qrc:/images/bkwd-icon.png"
                onClicked: {
                    songListView.decrementCurrentIndex()
                }
            }
            CustomButton {
                idle_background: "qrc:/images/player-background-idle.png"
                pressed_background: "qrc:/images/player-background-pressed.png"
                button_icon: (gstManager.state === 0) ? "qrc:/images/play-icon.png" : "qrc:/images/pause-icon.png"
                onClicked: {
                    if (gstManager.state === 0) {
                        gstManager.play(fileListModel.get_id_for_index((songListView.currentIndex)))
                    } else {
                        gstManager.pause()
                    }
                }
            }
            CustomButton {
                idle_background: "qrc:/images/player-background-idle.png"
                pressed_background: "qrc:/images/player-background-pressed.png"
                button_icon: "qrc:/images/fwd-icon.png"
                onClicked: {
                    songListView.incrementCurrentIndex()
                }
            }
        }
    }

    CustomButton {
        idle_background: "qrc:/images/menu-background-idle.png"
        pressed_background: "qrc:/images/menu-background-pressed.png"
        button_icon: "qrc:/images/menu-icon.png"
        anchors {
            left: music_player_ui.left
            bottom: music_player_ui.bottom
            bottomMargin: 10
            leftMargin: 10
        }
        onClicked: {
            mainLoader.source = "qrc:/main-menu-ui.qml"
        }
    }

    CustomButton {
        idle_background: "qrc:/images/menu-background-idle.png"
        pressed_background: "qrc:/images/menu-background-pressed.png"
        button_icon: "qrc:/images/folder-icon.png"
        anchors {
            right: music_player_ui.right
            bottom: music_player_ui.bottom
            bottomMargin: 10
            rightMargin: 10
        }
        onClicked: popup.open()
    }

    Popup {
        id: popup

        anchors.centerIn: parent
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            id: popup_background
            height: music_player_ui.height*0.8
            width: music_player_ui.width*0.8
            color: "black"
            border.width: 3
            border.color: "white"
            opacity: 0.75
            anchors.centerIn: parent
        }

        contentItem: ListView{
            id: popup_listview
            anchors.centerIn: parent
            implicitWidth: popup_background.width*0.9
            implicitHeight: popup_background.height*0.9

            model: folderListModel

            delegate: ItemDelegate {
                property string element_text: model.item_name
                width: popup_listview.width
                contentItem: Text {
                    font.pointSize: 16
                    color: "white"
                    text: model.item_name
                }

                onClicked: {
                    popup_listview.currentIndex = index
                    pathManager.change_folder(popup_listview.currentItem.element_text)
                    popup.close()
                }
            }
        }
    }

}
