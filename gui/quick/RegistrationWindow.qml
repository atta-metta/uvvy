import QtQuick 2.0
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.0
import QtQuick.Window 2.0
import QtQuick.Controls 1.0

Window
{
    title: "Registration"
    width: 300
    height: 600

    FileDialog {
        id: fileDialog
        visible: false
        modality: Qt.WindowModal
        title: "Choose Avatar"
        selectExisting: true
        selectMultiple: false
        selectFolder: false
        nameFilters: [ "Image files (*.png *.jpg)" ]
        selectedNameFilter: "Image files (*.png *.jpg)"
        sidebarVisible: false
        onAccepted: {
            avatar.text = fileUrls[0]
        }
    }

    Column {
        anchors.centerIn: parent
        Label {
            text: "Username"
        }
        TextField {
            id: username
            anchors.margins: 2

            maximumLength: 50
            focus: true
            horizontalAlignment: TextInput.AlignHCenter
            onAccepted: { password.focus = true }
        }

        Label {
            text: "Password"
        }
        TextField {
            id: password
            anchors.margins: 2
            focus: true
            echoMode : TextInput.Password
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { password2.focus = true }
        }
        Label {
            text: "Password Again"
        }
        TextField {
            id: password2
            anchors.margins: 2
            focus: true
            echoMode : TextInput.Password
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { email.focus = true }
        }

        Rectangle {
            width : 50
            height : 10
            border.width: 0
            color: "transparent"
        }

        Label {
            text: "Email"
        }

        TextField {
            id: email
            anchors.margins: 2
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { firstName.focus = true }
        }

        Rectangle {
            width : 50
            height : 10
            border.width: 0
            color: "transparent"
        }

        Label {
            text: "First Name"
        }

        TextField {
            id: firstName
            anchors.margins: 2
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { lastName.focus = true }
        }

        Rectangle {
            width : 50
            height : 10
            border.width: 0
            color: "transparent"
        }

        Label {
            text: "Last Name"
        }

        TextField {
            id: lastName
            anchors.margins: 2
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { city.focus = true }
        }

        Rectangle {
            width : 50
            height : 10
            border.width: 0
            color: "transparent"
        }

        Label {
            text: "City"
        }

        TextField {
            id: city
            anchors.margins: 2
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { avatar.focus = true }
        }

        Rectangle {
            width : 50
            height : 10
            border.width: 0
            color: "transparent"
        }

        Label {
            text: "Avatar"
        }

        TextField {
            id: avatar
            anchors.margins: 2
            horizontalAlignment: TextInput.AlignHCenter

            onAccepted: { registration_button.focus = true }

            onFocusChanged: {
                if (focus == true) {
                    fileDialog.visible = true
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (avatar.focus == true) {
                        fileDialog.visible = true
                    } else {
                        avatar.focus = true
                    }
                }
            }
        }

        Button {
            id: registration_button

            text: "Register"
            onClicked: { accountManager.registerUser(username.text, password.text, email.text,
                    firstName.text, lastName.text, city.text, avatar.text) }
        }

        Button {
            id: cancel

            text: "Cancel"
            onClicked: { windowManager.closeRegistrationWindow() }
        }
    }
}
