import QtQuick 1.1
import org.nemomobile.social 1.0

Item {
    id: container
    property Facebook model
    signal backClicked
    signal photoClicked(string photoId)
    anchors.fill: parent

    Text {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "You have " + model.count + " photos in the album"
    }

    Button {
        id: backButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Back"
        onClicked: container.backClicked()
    }

    GridView {
        id: view
        clip: true
        anchors.top: topLabel.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        cellWidth: width / 4
        cellHeight: cellWidth
        model: container.model
        delegate: MouseArea {
            id: photoDelegate
            onClicked: container.photoClicked(model.contentItem.identifier)
            width: view.cellWidth
            height: view.cellHeight
            clip: true
            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                source: model.contentItem.picture // low-res icon
            }
        }
    }
}
