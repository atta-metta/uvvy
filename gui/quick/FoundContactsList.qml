/* Contact List */
import QtQuick 2.3
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1

Window {
    id: foundContactsDialog
    visible: true
    title: "Found Contacts"
    width: 400
    height: 600

    Rectangle {
        anchors.fill: parent
        Column {
            anchors.fill: parent

            ListView {
                anchors.fill: parent
                delegate: ContactRequestDelegate {}
                model: foundContactsModel
                focus: true
                clip: true
            }
        }
    }
}
