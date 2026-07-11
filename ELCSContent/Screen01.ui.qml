

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

    palette.button: Constants.surface0
    palette.buttonText: Constants.text
    palette.text: Constants.text

    ColumnLayout {
        id: mainColumnLayout
        anchors.right: parent.right
        anchors.fill: parent
        spacing: 0
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
            height: 200
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            rightPadding: 0
            bottomPadding: 0
            padding: 12
            leftPadding: 0
            topPadding: 0
            Layout.fillHeight: true
            Layout.fillWidth: true

            Rectangle {
                id: homeRect
                color: Constants.surface0
                anchors.fill: parent
                anchors.leftMargin: 0
                anchors.rightMargin: 0
                anchors.topMargin: 0
                anchors.bottomMargin: 0

                Rectangle {
                    id: rectangle1
                    width: 422
                    color: Constants.mantle
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom

                    ColumnLayout {
                        id: columnLayout2
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        anchors.topMargin: 10
                        anchors.bottomMargin: 10
                        uniformCellSizes: false

                        Text {
                            id: text1
                            height: 158
                            text: qsTr("ELCS")
                            color: Constants.text
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            font.pixelSize: 50
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            font.styleName: "Semibold"
                        }

                        ColumnLayout {
                            id: columnLayout1
                            height: 887
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: scrollView.top
                            anchors.topMargin: 0
                            Layout.fillHeight: false
                            Layout.fillWidth: false

                            Rectangle {
                                id: rectangle8
                                height: 454
                                color: Constants.surface1
                                radius: Constants.cornerRounding
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: scrollView.top
                                anchors.bottomMargin: -576
                                layer.smooth: true

                                ScrollView {
                                    id: scrollView
                                    x: -8
                                    y: 52
                                    width: 422
                                    height: 402
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    anchors.leftMargin: 0
                                    anchors.rightMargin: 0

                                    ColumnLayout {
                                        id: columnLayout
                                    }
                                }

                                Text {
                                    id: text3
                                    y: 0
                                    height: 46
                                    color: Constants.text
                                    text: qsTr("Recent Projects")
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.bottom: scrollView.top
                                    anchors.topMargin: 0
                                    anchors.bottomMargin: 17
                                    font.pixelSize: 26
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.styleName: "Bold"
                                }

                                ToolSeparator {
                                    id: toolSeparator
                                    x: 0
                                    y: 33
                                    width: 406
                                    height: 13
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: scrollView.top
                                    anchors.leftMargin: 0
                                    anchors.rightMargin: 0
                                    anchors.bottomMargin: 6
                                    rightPadding: 10
                                    leftPadding: 10
                                    orientation: Qt.Horizontal
                                }
                            }
                        }

                        RoundButton {
                            id: openProjectButton
                            radius: Constants.cornerRounding
                            text: "Open Project"
                            anchors.left: parent.left
                            anchors.right: parent.right
                            checkable: false
                            rightInset: 0
                            leftInset: 0
                            font.bold: true
                            font.pointSize: 25
                            display: AbstractButton.TextOnly
                        }

                        RoundButton {
                            id: createProjectButton
                            radius: 10
                            text: "Create Project"
                            anchors.left: parent.left
                            anchors.right: parent.right
                            checkable: false
                            rightInset: 0
                            leftInset: 0
                            font.pointSize: 25
                            font.bold: true
                            display: AbstractButton.TextOnly
                        }

                        RoundButton {
                            id: settingsButton
                            radius: 10
                            text: "Settings"
                            anchors.left: parent.left
                            anchors.right: parent.right
                            checkable: false
                            rightInset: 0
                            leftInset: 0
                            font.pointSize: 25
                            font.bold: true
                            display: AbstractButton.TextOnly
                        }

                        RoundButton {
                            id: creditsButton
                            radius: 10
                            text: "Credits"
                            anchors.left: parent.left
                            anchors.right: parent.right
                            checkable: false
                            rightInset: 0
                            leftInset: 0
                            font.pointSize: 25
                            font.bold: true
                            display: AbstractButton.TextOnly
                        }
                    }
                }

                Text {
                    id: text2
                    x: 476
                    y: 1031
                    text: qsTr("Text")
                    font.pixelSize: 12
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
