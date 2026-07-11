

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick
import QtQuick.Controls
import ELCS
import QtQuick.Layouts

Rectangle {
    id: rectangle
    width: Constants.width
    height: Constants.height
    anchors.fill: parent
    visible: true

    color: Constants.backgroundColor
    transformOrigin: Item.Center
    scale: 1

    ColumnLayout {
        id: mainColumnLayout
        x: 0
        y: 0
        width: 1920
        height: 1080
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.topMargin: 0
        anchors.bottomMargin: 0
        anchors.horizontalCenterOffset: 0
        spacing: 0
        anchors.horizontalCenter: parent.horizontalCenter
        TabBar {
            id: buttonTab
            position: TabBar.Header
            wheelEnabled: true
            contentHeight: 40
            TabButton {
                id: homeButton
                x: 0
                width: 69
                height: 25
                text: qsTr("Home")
                anchors.verticalCenter: parent.verticalCenter

                Connections {
                    target: homeButton
                    function onToggled() {
                        homeRect.visible = true
                        simulatorRect.visible = false
                        learnRect.visible = false
                    }
                }
            }

            TabButton {
                id: simulatorButton
                x: 70
                width: 69
                height: 25
                text: qsTr("Sandbox")
                anchors.verticalCenter: parent.verticalCenter

                Connections {
                    target: simulatorButton
                    function onToggled() {
                        homeRect.visible = false
                        simulatorRect.visible = true
                        learnRect.visible = false
                    }
                }
            }

            TabButton {
                id: learnButton
                x: 140
                width: 69
                height: 25
                text: qsTr("Learn")
                anchors.verticalCenter: parent.verticalCenter

                Connections {
                    target: learnButton
                    function onToggled() {
                        homeRect.visible = false
                        simulatorRect.visible = false
                        learnRect.visible = true
                    }
                }
            }
            Layout.preferredWidth: 209
            Layout.preferredHeight: 25
        }

        Frame {
            id: pageFrame
            width: 207
            height: 200
            rightPadding: 0
            bottomPadding: 0
            padding: 12
            leftPadding: 0
            topPadding: 0
            Layout.fillHeight: true
            Layout.fillWidth: true

            Rectangle {
                id: homeRect
                anchors.fill: parent

                Rectangle {
                    id: rectangle1
                    x: 0
                    width: 422
                    color: Constants.backgroundColor
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom

                    Text {
                        id: text1
                        height: 162
                        text: qsTr("ELCS")
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        font.pixelSize: 50
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.styleName: "Semibold"
                    }

                    ScrollView {
                        id: scrollView
                        x: 0
                        y: 168
                        width: 422
                        height: 295
                        anchors.left: parent.left
                        anchors.right: parent.right

                        ColumnLayout {
                            id: columnLayout
                            anchors.fill: parent
                        }
                    }

                    Rectangle {
                        id: rectangle2
                        x: 0
                        y: 968
                        width: 422
                        height: 79
                        color: "#494949"
                        radius: 10
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: button.bottom
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.bottomMargin: -968
                        layer.smooth: true

                        Button {
                            id: button
                            text: qsTr("Credits")
                            anchors.fill: parent
                            anchors.topMargin: 10
                            anchors.bottomMargin: 10
                            rightInset: 0
                            leftInset: 0
                            topInset: 0
                            hoverEnabled: true
                            enabled: true
                            font.preferShaping: false
                            activeFocusOnTab: false
                            highlighted: false
                            flat: true
                            display: AbstractButton.TextOnly
                            icon.cache: false
                            bottomPadding: 8
                            topPadding: 8
                            font.pointSize: 25
                            checked: true
                            background.opacity: 0
                        }
                    }

                    Rectangle {
                        id: rectangle3
                        x: 0
                        y: 883
                        width: 422
                        height: 79
                        color: "#494949"
                        radius: 10
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: button1.bottom
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.bottomMargin: -916
                        layer.smooth: true
                        Button {
                            id: button1
                            text: qsTr("Settings")
                            anchors.fill: parent
                            anchors.topMargin: 10
                            anchors.bottomMargin: 10
                            topPadding: 8
                            topInset: 0
                            rightInset: 0
                            leftInset: 0
                            icon.cache: false
                            hoverEnabled: true
                            highlighted: false
                            font.preferShaping: false
                            font.pointSize: 25
                            flat: true
                            enabled: true
                            display: AbstractButton.TextOnly
                            checked: true
                            bottomPadding: 8
                            background.opacity: 0
                            activeFocusOnTab: false
                        }
                    }

                    Rectangle {
                        id: rectangle5
                        x: 0
                        y: 713
                        width: 422
                        height: 79
                        color: "#494949"
                        radius: 10
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: button3.bottom
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.bottomMargin: -746
                        layer.smooth: true
                        Button {
                            id: button3
                            text: qsTr("Create New Project")
                            anchors.fill: parent
                            anchors.leftMargin: 0
                            anchors.rightMargin: 0
                            anchors.topMargin: 12
                            anchors.bottomMargin: 8
                            topPadding: 8
                            topInset: 0
                            rightInset: 0
                            leftInset: 0
                            icon.cache: false
                            hoverEnabled: true
                            highlighted: false
                            font.preferShaping: false
                            font.pointSize: 25
                            flat: true
                            enabled: true
                            display: AbstractButton.TextOnly
                            checked: true
                            bottomPadding: 8
                            background.opacity: 0
                            activeFocusOnTab: false
                        }
                    }

                    Rectangle {
                        id: rectangle7
                        x: 0
                        y: 628
                        width: 422
                        height: 79
                        color: "#494949"
                        radius: 10
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: button5.bottom
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.bottomMargin: -661
                        layer.smooth: true
                        Button {
                            id: button5
                            text: qsTr("Open Project From File")
                            anchors.fill: parent
                            anchors.leftMargin: 0
                            anchors.rightMargin: 0
                            anchors.topMargin: 12
                            anchors.bottomMargin: 8
                            topPadding: 8
                            topInset: 0
                            rightInset: 0
                            leftInset: 0
                            icon.cache: false
                            hoverEnabled: true
                            highlighted: false
                            font.preferShaping: false
                            font.pointSize: 25
                            flat: true
                            enabled: true
                            display: AbstractButton.TextOnly
                            checked: true
                            bottomPadding: 8
                            background.opacity: 0
                            activeFocusOnTab: false
                        }
                    }

                    Rectangle {
                        id: rectangle8
                        x: 0
                        y: 168
                        width: 422
                        height: 454
                        color: "#494949"
                        radius: 10
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: scrollView.top
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.bottomMargin: -576
                        layer.smooth: true
                    }
                }
            }

            Rectangle {
                id: simulatorRect
                visible: false
                enabled: true
            }

            Rectangle {
                id: learnRect
                visible: false
                enabled: true
            }
        }
    }
    states: [
        State {
            name: "clicked"
        }
    ]
}
