import QtQuick 2.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0

Dialog {
    id: addContactDialog
    visible: true
    title: "Add Contact"
    width: 400
    height: 600

    Column {
        anchors.centerIn: parent
        Label {
            text: "Username/Name/Email"
        }
        TextField {
            id: username
            anchors.margins: 2

            maximumLength: 50
            focus: true
            horizontalAlignment: TextInput.AlignHCenter
        }
    }

    onAccepted: {
        accountManager.findContact(username.text)
        Qt.createQmlObject('FoundContactsList{}', main, 2)
    }
}
