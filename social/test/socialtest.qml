import QtQuick 1.1
import org.nemomobile.social 1.0
import Sailfish.Silica 1.0

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

ApplicationWindow {

    id: root
    property string accessToken // provided by main.cpp
    initialPage: MainPage {
        accessToken: root.accessToken
    }
}
