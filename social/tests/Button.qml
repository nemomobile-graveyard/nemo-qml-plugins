import QtQuick 1.1

Rectangle {
    id: container
    property alias text: text.text
    signal clicked()
    width: 200
    height: 60
    color: !mouseArea.pressed ? "white" : "#DCDCDC"

    Text {
        id: text
        anchors.centerIn: parent
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: container.clicked()
    }
}
