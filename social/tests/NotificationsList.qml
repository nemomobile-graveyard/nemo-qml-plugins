import QtQuick 1.1
import org.nemomobile.social 1.0

Item {
    id: container
    property Facebook model
    signal backClicked
    anchors.fill: parent

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "You have " + model.count + " notifications"
    }

    Button {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: container.backClicked()
    }

    ListView {
        id: view
        clip: true
        anchors.top: topLabel.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        model: container.model
        delegate: Item {
            id: notificationDelegate
            width: view.width
            height: childrenRect.height
            Column {
                width: parent.width
                height: nameLabel.height + countLabel.height
                Text {
                    id: nameLabel
                    text: "From: " + model.contentItem.from.objectName
                }
                Text {
                    id: countLabel
                    text: "Title: " + model.contentItem.title
                }
            }
        }
    }
}
