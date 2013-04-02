import QtQuick 1.1
import org.nemomobile.social 1.0

/*

    This is a manual test application.

    It consists of various views which make use of functionality
    which is provided by the social plugin.

    The first page consists of various options which may be selected.
    The options include:
        - show recent notifications
        - show friends
        - show all albums

    If the "notifications" option is selected, a listview with recent
    notifications is shown.

    If the "friends" option is selected, a listview with all of the
    user's friends is shown.

    If the "albums" option is selected, the user may then select an
    album whose photos can be shown, in a gridview.  Then, if the
    user selects a photo, the comments on that photo will be shown
    in a listview.
*/

Item {
    id: root
    width: 200
    height: 200
    property string accessToken // provided by main.cpp
    property int whichActive: 0

    Component.onCompleted: showPage(0)

    function makeActive(which, nodeId) {
        facebook.nodeIdentifier = nodeId
        switch (which) {
        case 1:
            facebook.filters = [ facebook.notificationsFilter ]
            break
        case 2:
            facebook.filters = [ facebook.friendsFilter ]
            break
        case 3:
            facebook.filters = [ facebook.albumsFilter ]
            break
        case 4:
            facebook.filters = [ facebook.photosFilter ]
            break
        case 5:
            facebook.filters = [ facebook.commentsFilter ]
            break
        }
        whichActive = which
    }

    Facebook {
        id: facebook
        accessToken: root.accessToken

        property QtObject notificationsFilter: ContentItemTypeFilter { type: Facebook.Notification; limit: 10 }
        property QtObject friendsFilter:       ContentItemTypeFilter { type: Facebook.User }
        property QtObject albumsFilter:        ContentItemTypeFilter { type: Facebook.Album }
        property QtObject photosFilter:        ContentItemTypeFilter { type: Facebook.Photo }
        property QtObject commentsFilter:      ContentItemTypeFilter { type: Facebook.Comment }
    }



    ListView {
        id: main
        visible: whichActive == 0
        anchors.fill: parent
        model: ListModel {
            ListElement {
                text: "Show notifications"
                which: 1
            }
            ListElement {
                text: "Show friends"
                which: 2
            }
            ListElement {
                text: "Show albums"
                which: 3
            }
            ListElement {
                text: "Quit"
                which: -1
            }
        }

        delegate: Item {
            width: main.width
            height: childrenRect.height
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: model.text
                onClicked: {
                    if (model.which == -1) {
                        Qt.quit()
                    } else {
                        makeActive(model.which, "me")
                    }
                }
            }
        }
    }

    NotificationsList {
        id: notificationsList
        visible: whichActive == 1
        model: visible ? facebook : null
        onBackClicked: makeActive(0, "me")
    }

    FriendsList {
        id: friendsList
        visible: whichActive == 2
        model: visible ? facebook : null
        onBackClicked: makeActive(0, "me")
    }

    AlbumsList {
        id: albumsList
        visible: whichActive == 3
        model: visible ? facebook : null
        onBackClicked: makeActive(0, "me")
        onAlbumClicked: makeActive(4, albumId)
    }

    PhotosGrid {
        id: photosGrid
        visible: whichActive == 4
        model: visible ? facebook : null
        onBackClicked: makeActive(3, "me")
        onPhotoClicked: {
            photoCommentsList.backAlbumId = model.node.identifier
            makeActive(5, photoId)
        }
    }

    PhotoCommentsList {
        id: photoCommentsList
        property string backAlbumId
        visible: whichActive == 5
        model: visible ? facebook : null
        onBackClicked: {
            makeActive(4, backAlbumId) // back to photos page
            facebook.nodeIdentifier = backAlbumId // shouldn't need this... force
        }

    }

}
