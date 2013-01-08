import QtQuick 1.1
import org.nemomobile.time 1.0

WallClock {
    property int year: time.getFullYear()
    property int month: time.getMonth()
    property int day: time.getDate()
    property int hour: time.getHours()
    property int minute: time.getMinutes()
    property int second: time.getSeconds()
    updateFrequency: WallClock.Second
}

