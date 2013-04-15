import QtQuick 1.1
import org.nemomobile.social 1.0
import Sailfish.Silica 1.0

Page {
    id: root

    property string accessToken // provided by main.cpp

    // ----------------

    property int whichActive: 0 // 0 = main page, 1 = notifications, 2 = friends, 3 = albums, 4 = photos, 5 = comments

    function makeActive(which, nodeId) {
        fb.nodeIdentifier = nodeId
        if (which == 1) {
            fb.filters = [ fb.notificationsFilter ]
        } else if (which == 2) {
            fb.filters = [ fb.friendsFilter ]
        } else if (which == 3) {
            fb.filters = [ fb.albumsFilter ]
        } else if (which == 4) {
            fb.filters = [ fb.photosFilter ]
        } else if (which == 5) {
            fb.filters = [ fb.commentsFilter ]
        }

        whichActive = which
    }

    Facebook {
        id: fb
        accessToken: root.accessToken

        property QtObject notificationsFilter: ContentItemTypeFilter { type: Facebook.Notification; limit: 10 }
        property QtObject friendsFilter:       ContentItemTypeFilter { type: Facebook.User }
        property QtObject albumsFilter:        ContentItemTypeFilter { type: Facebook.Album }
        property QtObject photosFilter:        ContentItemTypeFilter { type: Facebook.Photo }
        property QtObject commentsFilter:      ContentItemTypeFilter { type: Facebook.Comment }
    }

    Column {
        anchors.centerIn: parent
        width: parent.width
        height: childrenRect.height
        visible: whichActive == 0
        spacing: 20
        Button {
            text: "Show Notifications"
            onClicked: makeActive(1, "me")
        }
        Button {
            text: "Show Friends"
            onClicked: makeActive(2, "me")
        }
        Button {
            text: "Show Albums"
            onClicked: makeActive(3, "me")
        }
        Button {
            text: "Quit"
            onClicked: Qt.quit()
        }
    }

    NotificationsList {
        id: notifications
        anchors.fill: parent
        visible: whichActive == 1
        model: visible ? fb : null
        onBackClicked: makeActive(0, "me")
    }

    FriendsList {
        id: friends
        anchors.fill: parent
        visible: whichActive == 2
        model: visible ? fb : null
        onBackClicked: makeActive(0, "me")
    }

    AlbumsList {
        id: albums
        anchors.fill: parent
        visible: whichActive == 3
        model: visible ? fb : null
        onBackClicked: makeActive(0, "me")
        onAlbumClicked: {
            console.log("album clicked; setting node id to: " + albumId)
            makeActive(4, albumId)
        }
    }

    PhotosGrid {
        id: photos
        anchors.fill: parent
        visible: whichActive == 4
        model: visible ? fb : null
        onBackClicked: makeActive(3, "me") // back to albums page
        onPhotoClicked: {
            comments.backAlbumId = model.node.identifier
            console.log("photo clicked; setting back album id to: " + comments.backAlbumId)
            makeActive(5, photoId)
        }
    }

    PhotoCommentsList {
        id: comments
        property string backAlbumId
        anchors.fill: parent
        visible: whichActive == 5
        model: visible ? fb : null
        onBackClicked: {
            makeActive(4, backAlbumId) // back to photos page
            fb.nodeIdentifier = backAlbumId // shouldn't need this... force repopulate.
        }
    }
}
