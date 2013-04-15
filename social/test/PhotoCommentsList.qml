import QtQuick 1.1
import org.nemomobile.social 1.0
import Sailfish.Silica 1.0

Item {
    id: root

    property Facebook model

    signal backClicked

    Image {
        id: backgroundImage
        opacity: 0.4
        anchors.fill: parent
        source: model.node.source // full-size image url
    }

    Label {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Comments: have " + model.count + " comments on this photo"
    }

    Button {
        id: backBtn
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: root.backClicked()
    }

    SilicaFlickable {
        id: flicky

        anchors.top: topLabel.bottom
        anchors.bottom: backBtn.top
        anchors.left: parent.left
        anchors.right: parent.right

        ListView {
            id: view
            anchors.fill: parent
            model: root.model
            delegate: Item {
                id: commentDelegate
                width: parent.width
                Column {
                    width: parent.width
                    height: nameLabel.height + countLabel.height
                    Label {
                        id: nameLabel
                        text: "From: " + contentItem.from.objectName
                    }
                    Label {
                        id: countLabel
                        text: "Message: " + contentItem.message
                    }
                }
            }
        }
    }
}
