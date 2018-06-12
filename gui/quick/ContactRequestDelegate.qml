/* User card */
import QtQuick 2.1
import QtQuick.Controls 1.0

Component {
    Item {
        width: parent.width
        height: cardInfo.height
        Row {
            id: cardInfo
            spacing: 8
            Image {
                id: avatar
                source: avatarUrl
                width: 96
                height: 96
                smooth: true
            }
            Column {
                spacing: 3
                Text {
                    text: userName
                    font.pixelSize: 24
                }
                Text {
                    text: userName
                    font.pixelSize: 20
                }
                Text {
                    text: firstName
                    font.pixelSize: 18
                }
                Text {
                    text: lastName
                    font.pixelSize: 14
                }
                Text {
                    text: host
                    font.pixelSize: 10
                }
                Text {
                    text: city
                    font.pixelSize: 10
                }
                Text {
                    text: eid
                    font.pixelSize: 8
                }
            }
            Column {
                Button {
                    text: "Send Request"
                    onClicked: { accountManager.contactRequest(this_) }
                }
            }
        }
    }
}
