/* Contact List */
import QtQuick 2.3
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtQuick.Window 2.1


ApplicationWindow {
    id: main
    title: "Contacts"
    width: 400
    height: 600

    menuBar: MenuBar {
        Menu {
            title: "Uvvy"
            MenuItem {
                text: "Add Contact"
                action: Action { 
                    onTriggered: Qt.createQmlObject('AddContactDialog{}', main, 1)
                    shortcut: "Ctrl+N"
                }
            }
            MenuSeparator {
            }
            MenuItem {
                text: "Quit"
                action: Action { 
                    onTriggered: root.exit()
                    shortcut: "Ctrl+Q"
                }
            }
        }
        Menu {
            title: "Help"
            MenuItem {
                text : "Help"
            }
        }
    }
    Column {
        anchors.fill: parent
        ScrollView {
            anchors.fill: parent
            ListView {
                anchors.fill: parent
                delegate: UserInfoDelegate {}
                model: contactListModel
                focus: true
                clip: true
            }
        }
    }
}
