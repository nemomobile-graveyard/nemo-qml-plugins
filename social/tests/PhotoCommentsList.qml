import QtQuick 1.1
import org.nemomobile.social 1.0

Item {
    id: root
    property Facebook model
    signal backClicked
    anchors.fill: parent

    Image {
        id: backgroundImage
        opacity: 0.4
        anchors.fill: parent
        source: model.node.source // full-size image url
    }

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Comments: have " + model.count + " comments on this photo"
    }

    Button {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: root.backClicked()
    }

    ListView {
        id: view
        clip: true
        anchors.top: topLabel.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: root.model
        delegate: Item {
            id: commentDelegate
            width: parent.width
            height: childrenRect.height
            Column {
                width: parent.width
                height: nameLabel.height + countLabel.height
                Text {
                    id: nameLabel
                    text: "From: " + contentItem.from.objectName
                }
                Text {
                    id: countLabel
                    text: "Message: " + contentItem.message
                }
            }
        }
    }
}
