import QtQuick 1.1
import org.nemomobile.social 1.0
import Sailfish.Silica 1.0

Item {
    id: root

    property Facebook model

    signal backClicked
    signal photoClicked(string photoId)

    Label {
        id: topLabel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Photos: have " + model.count + " photos in the album"
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
        clip: true

        GridView {
            id: view
            cellWidth: root.width / 4
            cellHeight: cellWidth
            anchors.fill: parent
            model: root.model
            delegate: MouseArea {
                id: photoDelegate
                onClicked: root.photoClicked(contentItemIdentifier)
                width: view.cellWidth
                height: view.cellHeight
                clip: true
                Image {
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    source: contentItem.picture // low-res icon
                }
            }
        }
    }
}
