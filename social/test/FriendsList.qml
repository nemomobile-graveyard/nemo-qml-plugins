import QtQuick 1.1
import org.nemomobile.social 1.0
import Sailfish.Silica 1.0

Item {
    id: root

    property Facebook model

    signal backClicked

    Label {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Friends: have " + model.count + " friends"
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
                id: friendDelegate
                width: parent.width
                Column {
                    width: parent.width
                    height: nameLabel.height + countLabel.height
                    Label {
                        id: nameLabel
                        text: "Name: " + contentItem.name
                    }
                    Label {
                        id: countLabel
                        text: "Quotes: " + contentItem.quotes
                    }
                }
            }
        }
    }
}
